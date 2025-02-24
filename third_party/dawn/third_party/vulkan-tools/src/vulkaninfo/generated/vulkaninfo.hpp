
/*
 * Copyright (c) 2019-2022 The Khronos Group Inc.
 * Copyright (c) 2019-2022 Valve Corporation
 * Copyright (c) 2019-2022 LunarG, Inc.
 * Copyright (c) 2023-2024 RasterGrid Kft.
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
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

/*
 * This file is generated from the Khronos Vulkan XML API Registry.
 */

#include "vulkaninfo.h"
#include "outputprinter.h"

template <typename T>
std::string to_hex_str(const T i) {
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(sizeof(T)) << std::hex << i;
    return stream.str();
}

template <typename T>
std::string to_hex_str(Printer &p, const T i) {
    if (p.Type() == OutputType::json)
        return std::to_string(i);
    else if (p.Type() == OutputType::vkconfig_output)
        return std::string("\"") + to_hex_str(i) + std::string("\"");
    else
        return to_hex_str(i);
}

std::string StdVideoH264ChromaFormatIdcString(StdVideoH264ChromaFormatIdc value) {
    switch (value) {
        case STD_VIDEO_H264_CHROMA_FORMAT_IDC_MONOCHROME: return "STD_VIDEO_H264_CHROMA_FORMAT_IDC_MONOCHROME";
        case STD_VIDEO_H264_CHROMA_FORMAT_IDC_420: return "STD_VIDEO_H264_CHROMA_FORMAT_IDC_420";
        case STD_VIDEO_H264_CHROMA_FORMAT_IDC_422: return "STD_VIDEO_H264_CHROMA_FORMAT_IDC_422";
        case STD_VIDEO_H264_CHROMA_FORMAT_IDC_444: return "STD_VIDEO_H264_CHROMA_FORMAT_IDC_444";
        case STD_VIDEO_H264_CHROMA_FORMAT_IDC_INVALID: return "STD_VIDEO_H264_CHROMA_FORMAT_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264ChromaFormatIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH264ChromaFormatIdc(Printer &p, std::string name, StdVideoH264ChromaFormatIdc value) {
    p.PrintKeyString(name, StdVideoH264ChromaFormatIdcString(value));
}
std::string StdVideoH264ProfileIdcString(StdVideoH264ProfileIdc value) {
    switch (value) {
        case STD_VIDEO_H264_PROFILE_IDC_BASELINE: return "STD_VIDEO_H264_PROFILE_IDC_BASELINE";
        case STD_VIDEO_H264_PROFILE_IDC_MAIN: return "STD_VIDEO_H264_PROFILE_IDC_MAIN";
        case STD_VIDEO_H264_PROFILE_IDC_HIGH: return "STD_VIDEO_H264_PROFILE_IDC_HIGH";
        case STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE: return "STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE";
        case STD_VIDEO_H264_PROFILE_IDC_INVALID: return "STD_VIDEO_H264_PROFILE_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264ProfileIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH264ProfileIdc(Printer &p, std::string name, StdVideoH264ProfileIdc value) {
    p.PrintKeyString(name, StdVideoH264ProfileIdcString(value));
}
std::string StdVideoH264LevelIdcString(StdVideoH264LevelIdc value) {
    switch (value) {
        case STD_VIDEO_H264_LEVEL_IDC_1_0: return "STD_VIDEO_H264_LEVEL_IDC_1_0";
        case STD_VIDEO_H264_LEVEL_IDC_1_1: return "STD_VIDEO_H264_LEVEL_IDC_1_1";
        case STD_VIDEO_H264_LEVEL_IDC_1_2: return "STD_VIDEO_H264_LEVEL_IDC_1_2";
        case STD_VIDEO_H264_LEVEL_IDC_1_3: return "STD_VIDEO_H264_LEVEL_IDC_1_3";
        case STD_VIDEO_H264_LEVEL_IDC_2_0: return "STD_VIDEO_H264_LEVEL_IDC_2_0";
        case STD_VIDEO_H264_LEVEL_IDC_2_1: return "STD_VIDEO_H264_LEVEL_IDC_2_1";
        case STD_VIDEO_H264_LEVEL_IDC_2_2: return "STD_VIDEO_H264_LEVEL_IDC_2_2";
        case STD_VIDEO_H264_LEVEL_IDC_3_0: return "STD_VIDEO_H264_LEVEL_IDC_3_0";
        case STD_VIDEO_H264_LEVEL_IDC_3_1: return "STD_VIDEO_H264_LEVEL_IDC_3_1";
        case STD_VIDEO_H264_LEVEL_IDC_3_2: return "STD_VIDEO_H264_LEVEL_IDC_3_2";
        case STD_VIDEO_H264_LEVEL_IDC_4_0: return "STD_VIDEO_H264_LEVEL_IDC_4_0";
        case STD_VIDEO_H264_LEVEL_IDC_4_1: return "STD_VIDEO_H264_LEVEL_IDC_4_1";
        case STD_VIDEO_H264_LEVEL_IDC_4_2: return "STD_VIDEO_H264_LEVEL_IDC_4_2";
        case STD_VIDEO_H264_LEVEL_IDC_5_0: return "STD_VIDEO_H264_LEVEL_IDC_5_0";
        case STD_VIDEO_H264_LEVEL_IDC_5_1: return "STD_VIDEO_H264_LEVEL_IDC_5_1";
        case STD_VIDEO_H264_LEVEL_IDC_5_2: return "STD_VIDEO_H264_LEVEL_IDC_5_2";
        case STD_VIDEO_H264_LEVEL_IDC_6_0: return "STD_VIDEO_H264_LEVEL_IDC_6_0";
        case STD_VIDEO_H264_LEVEL_IDC_6_1: return "STD_VIDEO_H264_LEVEL_IDC_6_1";
        case STD_VIDEO_H264_LEVEL_IDC_6_2: return "STD_VIDEO_H264_LEVEL_IDC_6_2";
        case STD_VIDEO_H264_LEVEL_IDC_INVALID: return "STD_VIDEO_H264_LEVEL_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264LevelIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH264LevelIdc(Printer &p, std::string name, StdVideoH264LevelIdc value) {
    p.PrintKeyString(name, StdVideoH264LevelIdcString(value));
}
std::string StdVideoH264PocTypeString(StdVideoH264PocType value) {
    switch (value) {
        case STD_VIDEO_H264_POC_TYPE_0: return "STD_VIDEO_H264_POC_TYPE_0";
        case STD_VIDEO_H264_POC_TYPE_1: return "STD_VIDEO_H264_POC_TYPE_1";
        case STD_VIDEO_H264_POC_TYPE_2: return "STD_VIDEO_H264_POC_TYPE_2";
        case STD_VIDEO_H264_POC_TYPE_INVALID: return "STD_VIDEO_H264_POC_TYPE_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264PocType_value") + std::to_string(value);
    }
}
void DumpStdVideoH264PocType(Printer &p, std::string name, StdVideoH264PocType value) {
    p.PrintKeyString(name, StdVideoH264PocTypeString(value));
}
std::string StdVideoH264AspectRatioIdcString(StdVideoH264AspectRatioIdc value) {
    switch (value) {
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_UNSPECIFIED: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_UNSPECIFIED";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_SQUARE: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_SQUARE";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_12_11: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_12_11";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_10_11: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_10_11";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_16_11: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_16_11";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_40_33: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_40_33";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_24_11: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_24_11";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_20_11: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_20_11";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_32_11: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_32_11";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_80_33: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_80_33";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_18_11: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_18_11";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_15_11: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_15_11";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_64_33: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_64_33";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_160_99: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_160_99";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_4_3: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_4_3";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_3_2: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_3_2";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_2_1: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_2_1";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_EXTENDED_SAR: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_EXTENDED_SAR";
        case STD_VIDEO_H264_ASPECT_RATIO_IDC_INVALID: return "STD_VIDEO_H264_ASPECT_RATIO_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264AspectRatioIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH264AspectRatioIdc(Printer &p, std::string name, StdVideoH264AspectRatioIdc value) {
    p.PrintKeyString(name, StdVideoH264AspectRatioIdcString(value));
}
std::string StdVideoH264WeightedBipredIdcString(StdVideoH264WeightedBipredIdc value) {
    switch (value) {
        case STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_DEFAULT: return "STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_DEFAULT";
        case STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT: return "STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT";
        case STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_IMPLICIT: return "STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_IMPLICIT";
        case STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_INVALID: return "STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264WeightedBipredIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH264WeightedBipredIdc(Printer &p, std::string name, StdVideoH264WeightedBipredIdc value) {
    p.PrintKeyString(name, StdVideoH264WeightedBipredIdcString(value));
}
std::string StdVideoH264ModificationOfPicNumsIdcString(StdVideoH264ModificationOfPicNumsIdc value) {
    switch (value) {
        case STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_SHORT_TERM_SUBTRACT: return "STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_SHORT_TERM_SUBTRACT";
        case STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_SHORT_TERM_ADD: return "STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_SHORT_TERM_ADD";
        case STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_LONG_TERM: return "STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_LONG_TERM";
        case STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_END: return "STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_END";
        case STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_INVALID: return "STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264ModificationOfPicNumsIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH264ModificationOfPicNumsIdc(Printer &p, std::string name, StdVideoH264ModificationOfPicNumsIdc value) {
    p.PrintKeyString(name, StdVideoH264ModificationOfPicNumsIdcString(value));
}
std::string StdVideoH264MemMgmtControlOpString(StdVideoH264MemMgmtControlOp value) {
    switch (value) {
        case STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_END: return "STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_END";
        case STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_SHORT_TERM: return "STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_SHORT_TERM";
        case STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_LONG_TERM: return "STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_LONG_TERM";
        case STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_MARK_LONG_TERM: return "STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_MARK_LONG_TERM";
        case STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_SET_MAX_LONG_TERM_INDEX: return "STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_SET_MAX_LONG_TERM_INDEX";
        case STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_ALL: return "STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_ALL";
        case STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_MARK_CURRENT_AS_LONG_TERM: return "STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_MARK_CURRENT_AS_LONG_TERM";
        case STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_INVALID: return "STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264MemMgmtControlOp_value") + std::to_string(value);
    }
}
void DumpStdVideoH264MemMgmtControlOp(Printer &p, std::string name, StdVideoH264MemMgmtControlOp value) {
    p.PrintKeyString(name, StdVideoH264MemMgmtControlOpString(value));
}
std::string StdVideoH264CabacInitIdcString(StdVideoH264CabacInitIdc value) {
    switch (value) {
        case STD_VIDEO_H264_CABAC_INIT_IDC_0: return "STD_VIDEO_H264_CABAC_INIT_IDC_0";
        case STD_VIDEO_H264_CABAC_INIT_IDC_1: return "STD_VIDEO_H264_CABAC_INIT_IDC_1";
        case STD_VIDEO_H264_CABAC_INIT_IDC_2: return "STD_VIDEO_H264_CABAC_INIT_IDC_2";
        case STD_VIDEO_H264_CABAC_INIT_IDC_INVALID: return "STD_VIDEO_H264_CABAC_INIT_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264CabacInitIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH264CabacInitIdc(Printer &p, std::string name, StdVideoH264CabacInitIdc value) {
    p.PrintKeyString(name, StdVideoH264CabacInitIdcString(value));
}
std::string StdVideoH264DisableDeblockingFilterIdcString(StdVideoH264DisableDeblockingFilterIdc value) {
    switch (value) {
        case STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_DISABLED: return "STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_DISABLED";
        case STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_ENABLED: return "STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_ENABLED";
        case STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_PARTIAL: return "STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_PARTIAL";
        case STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_INVALID: return "STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264DisableDeblockingFilterIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH264DisableDeblockingFilterIdc(Printer &p, std::string name, StdVideoH264DisableDeblockingFilterIdc value) {
    p.PrintKeyString(name, StdVideoH264DisableDeblockingFilterIdcString(value));
}
std::string StdVideoH264SliceTypeString(StdVideoH264SliceType value) {
    switch (value) {
        case STD_VIDEO_H264_SLICE_TYPE_P: return "STD_VIDEO_H264_SLICE_TYPE_P";
        case STD_VIDEO_H264_SLICE_TYPE_B: return "STD_VIDEO_H264_SLICE_TYPE_B";
        case STD_VIDEO_H264_SLICE_TYPE_I: return "STD_VIDEO_H264_SLICE_TYPE_I";
        case STD_VIDEO_H264_SLICE_TYPE_INVALID: return "STD_VIDEO_H264_SLICE_TYPE_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264SliceType_value") + std::to_string(value);
    }
}
void DumpStdVideoH264SliceType(Printer &p, std::string name, StdVideoH264SliceType value) {
    p.PrintKeyString(name, StdVideoH264SliceTypeString(value));
}
std::string StdVideoH264PictureTypeString(StdVideoH264PictureType value) {
    switch (value) {
        case STD_VIDEO_H264_PICTURE_TYPE_P: return "STD_VIDEO_H264_PICTURE_TYPE_P";
        case STD_VIDEO_H264_PICTURE_TYPE_B: return "STD_VIDEO_H264_PICTURE_TYPE_B";
        case STD_VIDEO_H264_PICTURE_TYPE_I: return "STD_VIDEO_H264_PICTURE_TYPE_I";
        case STD_VIDEO_H264_PICTURE_TYPE_IDR: return "STD_VIDEO_H264_PICTURE_TYPE_IDR";
        case STD_VIDEO_H264_PICTURE_TYPE_INVALID: return "STD_VIDEO_H264_PICTURE_TYPE_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264PictureType_value") + std::to_string(value);
    }
}
void DumpStdVideoH264PictureType(Printer &p, std::string name, StdVideoH264PictureType value) {
    p.PrintKeyString(name, StdVideoH264PictureTypeString(value));
}
std::string StdVideoH264NonVclNaluTypeString(StdVideoH264NonVclNaluType value) {
    switch (value) {
        case STD_VIDEO_H264_NON_VCL_NALU_TYPE_SPS: return "STD_VIDEO_H264_NON_VCL_NALU_TYPE_SPS";
        case STD_VIDEO_H264_NON_VCL_NALU_TYPE_PPS: return "STD_VIDEO_H264_NON_VCL_NALU_TYPE_PPS";
        case STD_VIDEO_H264_NON_VCL_NALU_TYPE_AUD: return "STD_VIDEO_H264_NON_VCL_NALU_TYPE_AUD";
        case STD_VIDEO_H264_NON_VCL_NALU_TYPE_PREFIX: return "STD_VIDEO_H264_NON_VCL_NALU_TYPE_PREFIX";
        case STD_VIDEO_H264_NON_VCL_NALU_TYPE_END_OF_SEQUENCE: return "STD_VIDEO_H264_NON_VCL_NALU_TYPE_END_OF_SEQUENCE";
        case STD_VIDEO_H264_NON_VCL_NALU_TYPE_END_OF_STREAM: return "STD_VIDEO_H264_NON_VCL_NALU_TYPE_END_OF_STREAM";
        case STD_VIDEO_H264_NON_VCL_NALU_TYPE_PRECODED: return "STD_VIDEO_H264_NON_VCL_NALU_TYPE_PRECODED";
        case STD_VIDEO_H264_NON_VCL_NALU_TYPE_INVALID: return "STD_VIDEO_H264_NON_VCL_NALU_TYPE_INVALID";
        default: return std::string("UNKNOWN_StdVideoH264NonVclNaluType_value") + std::to_string(value);
    }
}
void DumpStdVideoH264NonVclNaluType(Printer &p, std::string name, StdVideoH264NonVclNaluType value) {
    p.PrintKeyString(name, StdVideoH264NonVclNaluTypeString(value));
}
std::string StdVideoDecodeH264FieldOrderCountString(StdVideoDecodeH264FieldOrderCount value) {
    switch (value) {
        case STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_TOP: return "STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_TOP";
        case STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_BOTTOM: return "STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_BOTTOM";
        case STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_INVALID: return "STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_INVALID";
        default: return std::string("UNKNOWN_StdVideoDecodeH264FieldOrderCount_value") + std::to_string(value);
    }
}
void DumpStdVideoDecodeH264FieldOrderCount(Printer &p, std::string name, StdVideoDecodeH264FieldOrderCount value) {
    p.PrintKeyString(name, StdVideoDecodeH264FieldOrderCountString(value));
}
std::string StdVideoH265ChromaFormatIdcString(StdVideoH265ChromaFormatIdc value) {
    switch (value) {
        case STD_VIDEO_H265_CHROMA_FORMAT_IDC_MONOCHROME: return "STD_VIDEO_H265_CHROMA_FORMAT_IDC_MONOCHROME";
        case STD_VIDEO_H265_CHROMA_FORMAT_IDC_420: return "STD_VIDEO_H265_CHROMA_FORMAT_IDC_420";
        case STD_VIDEO_H265_CHROMA_FORMAT_IDC_422: return "STD_VIDEO_H265_CHROMA_FORMAT_IDC_422";
        case STD_VIDEO_H265_CHROMA_FORMAT_IDC_444: return "STD_VIDEO_H265_CHROMA_FORMAT_IDC_444";
        case STD_VIDEO_H265_CHROMA_FORMAT_IDC_INVALID: return "STD_VIDEO_H265_CHROMA_FORMAT_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH265ChromaFormatIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH265ChromaFormatIdc(Printer &p, std::string name, StdVideoH265ChromaFormatIdc value) {
    p.PrintKeyString(name, StdVideoH265ChromaFormatIdcString(value));
}
std::string StdVideoH265ProfileIdcString(StdVideoH265ProfileIdc value) {
    switch (value) {
        case STD_VIDEO_H265_PROFILE_IDC_MAIN: return "STD_VIDEO_H265_PROFILE_IDC_MAIN";
        case STD_VIDEO_H265_PROFILE_IDC_MAIN_10: return "STD_VIDEO_H265_PROFILE_IDC_MAIN_10";
        case STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE: return "STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE";
        case STD_VIDEO_H265_PROFILE_IDC_FORMAT_RANGE_EXTENSIONS: return "STD_VIDEO_H265_PROFILE_IDC_FORMAT_RANGE_EXTENSIONS";
        case STD_VIDEO_H265_PROFILE_IDC_SCC_EXTENSIONS: return "STD_VIDEO_H265_PROFILE_IDC_SCC_EXTENSIONS";
        case STD_VIDEO_H265_PROFILE_IDC_INVALID: return "STD_VIDEO_H265_PROFILE_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH265ProfileIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH265ProfileIdc(Printer &p, std::string name, StdVideoH265ProfileIdc value) {
    p.PrintKeyString(name, StdVideoH265ProfileIdcString(value));
}
std::string StdVideoH265LevelIdcString(StdVideoH265LevelIdc value) {
    switch (value) {
        case STD_VIDEO_H265_LEVEL_IDC_1_0: return "STD_VIDEO_H265_LEVEL_IDC_1_0";
        case STD_VIDEO_H265_LEVEL_IDC_2_0: return "STD_VIDEO_H265_LEVEL_IDC_2_0";
        case STD_VIDEO_H265_LEVEL_IDC_2_1: return "STD_VIDEO_H265_LEVEL_IDC_2_1";
        case STD_VIDEO_H265_LEVEL_IDC_3_0: return "STD_VIDEO_H265_LEVEL_IDC_3_0";
        case STD_VIDEO_H265_LEVEL_IDC_3_1: return "STD_VIDEO_H265_LEVEL_IDC_3_1";
        case STD_VIDEO_H265_LEVEL_IDC_4_0: return "STD_VIDEO_H265_LEVEL_IDC_4_0";
        case STD_VIDEO_H265_LEVEL_IDC_4_1: return "STD_VIDEO_H265_LEVEL_IDC_4_1";
        case STD_VIDEO_H265_LEVEL_IDC_5_0: return "STD_VIDEO_H265_LEVEL_IDC_5_0";
        case STD_VIDEO_H265_LEVEL_IDC_5_1: return "STD_VIDEO_H265_LEVEL_IDC_5_1";
        case STD_VIDEO_H265_LEVEL_IDC_5_2: return "STD_VIDEO_H265_LEVEL_IDC_5_2";
        case STD_VIDEO_H265_LEVEL_IDC_6_0: return "STD_VIDEO_H265_LEVEL_IDC_6_0";
        case STD_VIDEO_H265_LEVEL_IDC_6_1: return "STD_VIDEO_H265_LEVEL_IDC_6_1";
        case STD_VIDEO_H265_LEVEL_IDC_6_2: return "STD_VIDEO_H265_LEVEL_IDC_6_2";
        case STD_VIDEO_H265_LEVEL_IDC_INVALID: return "STD_VIDEO_H265_LEVEL_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH265LevelIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH265LevelIdc(Printer &p, std::string name, StdVideoH265LevelIdc value) {
    p.PrintKeyString(name, StdVideoH265LevelIdcString(value));
}
std::string StdVideoH265SliceTypeString(StdVideoH265SliceType value) {
    switch (value) {
        case STD_VIDEO_H265_SLICE_TYPE_B: return "STD_VIDEO_H265_SLICE_TYPE_B";
        case STD_VIDEO_H265_SLICE_TYPE_P: return "STD_VIDEO_H265_SLICE_TYPE_P";
        case STD_VIDEO_H265_SLICE_TYPE_I: return "STD_VIDEO_H265_SLICE_TYPE_I";
        case STD_VIDEO_H265_SLICE_TYPE_INVALID: return "STD_VIDEO_H265_SLICE_TYPE_INVALID";
        default: return std::string("UNKNOWN_StdVideoH265SliceType_value") + std::to_string(value);
    }
}
void DumpStdVideoH265SliceType(Printer &p, std::string name, StdVideoH265SliceType value) {
    p.PrintKeyString(name, StdVideoH265SliceTypeString(value));
}
std::string StdVideoH265PictureTypeString(StdVideoH265PictureType value) {
    switch (value) {
        case STD_VIDEO_H265_PICTURE_TYPE_P: return "STD_VIDEO_H265_PICTURE_TYPE_P";
        case STD_VIDEO_H265_PICTURE_TYPE_B: return "STD_VIDEO_H265_PICTURE_TYPE_B";
        case STD_VIDEO_H265_PICTURE_TYPE_I: return "STD_VIDEO_H265_PICTURE_TYPE_I";
        case STD_VIDEO_H265_PICTURE_TYPE_IDR: return "STD_VIDEO_H265_PICTURE_TYPE_IDR";
        case STD_VIDEO_H265_PICTURE_TYPE_INVALID: return "STD_VIDEO_H265_PICTURE_TYPE_INVALID";
        default: return std::string("UNKNOWN_StdVideoH265PictureType_value") + std::to_string(value);
    }
}
void DumpStdVideoH265PictureType(Printer &p, std::string name, StdVideoH265PictureType value) {
    p.PrintKeyString(name, StdVideoH265PictureTypeString(value));
}
std::string StdVideoH265AspectRatioIdcString(StdVideoH265AspectRatioIdc value) {
    switch (value) {
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_UNSPECIFIED: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_UNSPECIFIED";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_SQUARE: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_SQUARE";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_12_11: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_12_11";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_10_11: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_10_11";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_16_11: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_16_11";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_40_33: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_40_33";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_24_11: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_24_11";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_20_11: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_20_11";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_32_11: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_32_11";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_80_33: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_80_33";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_18_11: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_18_11";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_15_11: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_15_11";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_64_33: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_64_33";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_160_99: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_160_99";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_4_3: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_4_3";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_3_2: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_3_2";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_2_1: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_2_1";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_EXTENDED_SAR: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_EXTENDED_SAR";
        case STD_VIDEO_H265_ASPECT_RATIO_IDC_INVALID: return "STD_VIDEO_H265_ASPECT_RATIO_IDC_INVALID";
        default: return std::string("UNKNOWN_StdVideoH265AspectRatioIdc_value") + std::to_string(value);
    }
}
void DumpStdVideoH265AspectRatioIdc(Printer &p, std::string name, StdVideoH265AspectRatioIdc value) {
    p.PrintKeyString(name, StdVideoH265AspectRatioIdcString(value));
}
std::string StdVideoAV1ProfileString(StdVideoAV1Profile value) {
    switch (value) {
        case STD_VIDEO_AV1_PROFILE_MAIN: return "STD_VIDEO_AV1_PROFILE_MAIN";
        case STD_VIDEO_AV1_PROFILE_HIGH: return "STD_VIDEO_AV1_PROFILE_HIGH";
        case STD_VIDEO_AV1_PROFILE_PROFESSIONAL: return "STD_VIDEO_AV1_PROFILE_PROFESSIONAL";
        case STD_VIDEO_AV1_PROFILE_INVALID: return "STD_VIDEO_AV1_PROFILE_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1Profile_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1Profile(Printer &p, std::string name, StdVideoAV1Profile value) {
    p.PrintKeyString(name, StdVideoAV1ProfileString(value));
}
std::string StdVideoAV1LevelString(StdVideoAV1Level value) {
    switch (value) {
        case STD_VIDEO_AV1_LEVEL_2_0: return "STD_VIDEO_AV1_LEVEL_2_0";
        case STD_VIDEO_AV1_LEVEL_2_1: return "STD_VIDEO_AV1_LEVEL_2_1";
        case STD_VIDEO_AV1_LEVEL_2_2: return "STD_VIDEO_AV1_LEVEL_2_2";
        case STD_VIDEO_AV1_LEVEL_2_3: return "STD_VIDEO_AV1_LEVEL_2_3";
        case STD_VIDEO_AV1_LEVEL_3_0: return "STD_VIDEO_AV1_LEVEL_3_0";
        case STD_VIDEO_AV1_LEVEL_3_1: return "STD_VIDEO_AV1_LEVEL_3_1";
        case STD_VIDEO_AV1_LEVEL_3_2: return "STD_VIDEO_AV1_LEVEL_3_2";
        case STD_VIDEO_AV1_LEVEL_3_3: return "STD_VIDEO_AV1_LEVEL_3_3";
        case STD_VIDEO_AV1_LEVEL_4_0: return "STD_VIDEO_AV1_LEVEL_4_0";
        case STD_VIDEO_AV1_LEVEL_4_1: return "STD_VIDEO_AV1_LEVEL_4_1";
        case STD_VIDEO_AV1_LEVEL_4_2: return "STD_VIDEO_AV1_LEVEL_4_2";
        case STD_VIDEO_AV1_LEVEL_4_3: return "STD_VIDEO_AV1_LEVEL_4_3";
        case STD_VIDEO_AV1_LEVEL_5_0: return "STD_VIDEO_AV1_LEVEL_5_0";
        case STD_VIDEO_AV1_LEVEL_5_1: return "STD_VIDEO_AV1_LEVEL_5_1";
        case STD_VIDEO_AV1_LEVEL_5_2: return "STD_VIDEO_AV1_LEVEL_5_2";
        case STD_VIDEO_AV1_LEVEL_5_3: return "STD_VIDEO_AV1_LEVEL_5_3";
        case STD_VIDEO_AV1_LEVEL_6_0: return "STD_VIDEO_AV1_LEVEL_6_0";
        case STD_VIDEO_AV1_LEVEL_6_1: return "STD_VIDEO_AV1_LEVEL_6_1";
        case STD_VIDEO_AV1_LEVEL_6_2: return "STD_VIDEO_AV1_LEVEL_6_2";
        case STD_VIDEO_AV1_LEVEL_6_3: return "STD_VIDEO_AV1_LEVEL_6_3";
        case STD_VIDEO_AV1_LEVEL_7_0: return "STD_VIDEO_AV1_LEVEL_7_0";
        case STD_VIDEO_AV1_LEVEL_7_1: return "STD_VIDEO_AV1_LEVEL_7_1";
        case STD_VIDEO_AV1_LEVEL_7_2: return "STD_VIDEO_AV1_LEVEL_7_2";
        case STD_VIDEO_AV1_LEVEL_7_3: return "STD_VIDEO_AV1_LEVEL_7_3";
        case STD_VIDEO_AV1_LEVEL_INVALID: return "STD_VIDEO_AV1_LEVEL_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1Level_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1Level(Printer &p, std::string name, StdVideoAV1Level value) {
    p.PrintKeyString(name, StdVideoAV1LevelString(value));
}
std::string StdVideoAV1FrameTypeString(StdVideoAV1FrameType value) {
    switch (value) {
        case STD_VIDEO_AV1_FRAME_TYPE_KEY: return "STD_VIDEO_AV1_FRAME_TYPE_KEY";
        case STD_VIDEO_AV1_FRAME_TYPE_INTER: return "STD_VIDEO_AV1_FRAME_TYPE_INTER";
        case STD_VIDEO_AV1_FRAME_TYPE_INTRA_ONLY: return "STD_VIDEO_AV1_FRAME_TYPE_INTRA_ONLY";
        case STD_VIDEO_AV1_FRAME_TYPE_SWITCH: return "STD_VIDEO_AV1_FRAME_TYPE_SWITCH";
        case STD_VIDEO_AV1_FRAME_TYPE_INVALID: return "STD_VIDEO_AV1_FRAME_TYPE_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1FrameType_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1FrameType(Printer &p, std::string name, StdVideoAV1FrameType value) {
    p.PrintKeyString(name, StdVideoAV1FrameTypeString(value));
}
std::string StdVideoAV1ReferenceNameString(StdVideoAV1ReferenceName value) {
    switch (value) {
        case STD_VIDEO_AV1_REFERENCE_NAME_INTRA_FRAME: return "STD_VIDEO_AV1_REFERENCE_NAME_INTRA_FRAME";
        case STD_VIDEO_AV1_REFERENCE_NAME_LAST_FRAME: return "STD_VIDEO_AV1_REFERENCE_NAME_LAST_FRAME";
        case STD_VIDEO_AV1_REFERENCE_NAME_LAST2_FRAME: return "STD_VIDEO_AV1_REFERENCE_NAME_LAST2_FRAME";
        case STD_VIDEO_AV1_REFERENCE_NAME_LAST3_FRAME: return "STD_VIDEO_AV1_REFERENCE_NAME_LAST3_FRAME";
        case STD_VIDEO_AV1_REFERENCE_NAME_GOLDEN_FRAME: return "STD_VIDEO_AV1_REFERENCE_NAME_GOLDEN_FRAME";
        case STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME: return "STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME";
        case STD_VIDEO_AV1_REFERENCE_NAME_ALTREF2_FRAME: return "STD_VIDEO_AV1_REFERENCE_NAME_ALTREF2_FRAME";
        case STD_VIDEO_AV1_REFERENCE_NAME_ALTREF_FRAME: return "STD_VIDEO_AV1_REFERENCE_NAME_ALTREF_FRAME";
        case STD_VIDEO_AV1_REFERENCE_NAME_INVALID: return "STD_VIDEO_AV1_REFERENCE_NAME_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1ReferenceName_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1ReferenceName(Printer &p, std::string name, StdVideoAV1ReferenceName value) {
    p.PrintKeyString(name, StdVideoAV1ReferenceNameString(value));
}
std::string StdVideoAV1InterpolationFilterString(StdVideoAV1InterpolationFilter value) {
    switch (value) {
        case STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP: return "STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP";
        case STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP_SMOOTH: return "STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP_SMOOTH";
        case STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP_SHARP: return "STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP_SHARP";
        case STD_VIDEO_AV1_INTERPOLATION_FILTER_BILINEAR: return "STD_VIDEO_AV1_INTERPOLATION_FILTER_BILINEAR";
        case STD_VIDEO_AV1_INTERPOLATION_FILTER_SWITCHABLE: return "STD_VIDEO_AV1_INTERPOLATION_FILTER_SWITCHABLE";
        case STD_VIDEO_AV1_INTERPOLATION_FILTER_INVALID: return "STD_VIDEO_AV1_INTERPOLATION_FILTER_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1InterpolationFilter_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1InterpolationFilter(Printer &p, std::string name, StdVideoAV1InterpolationFilter value) {
    p.PrintKeyString(name, StdVideoAV1InterpolationFilterString(value));
}
std::string StdVideoAV1TxModeString(StdVideoAV1TxMode value) {
    switch (value) {
        case STD_VIDEO_AV1_TX_MODE_ONLY_4X4: return "STD_VIDEO_AV1_TX_MODE_ONLY_4X4";
        case STD_VIDEO_AV1_TX_MODE_LARGEST: return "STD_VIDEO_AV1_TX_MODE_LARGEST";
        case STD_VIDEO_AV1_TX_MODE_SELECT: return "STD_VIDEO_AV1_TX_MODE_SELECT";
        case STD_VIDEO_AV1_TX_MODE_INVALID: return "STD_VIDEO_AV1_TX_MODE_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1TxMode_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1TxMode(Printer &p, std::string name, StdVideoAV1TxMode value) {
    p.PrintKeyString(name, StdVideoAV1TxModeString(value));
}
std::string StdVideoAV1FrameRestorationTypeString(StdVideoAV1FrameRestorationType value) {
    switch (value) {
        case STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_NONE: return "STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_NONE";
        case STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_WIENER: return "STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_WIENER";
        case STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_SGRPROJ: return "STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_SGRPROJ";
        case STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_SWITCHABLE: return "STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_SWITCHABLE";
        case STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_INVALID: return "STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1FrameRestorationType_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1FrameRestorationType(Printer &p, std::string name, StdVideoAV1FrameRestorationType value) {
    p.PrintKeyString(name, StdVideoAV1FrameRestorationTypeString(value));
}
std::string StdVideoAV1ColorPrimariesString(StdVideoAV1ColorPrimaries value) {
    switch (value) {
        case STD_VIDEO_AV1_COLOR_PRIMARIES_BT_709: return "STD_VIDEO_AV1_COLOR_PRIMARIES_BT_709";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_UNSPECIFIED: return "STD_VIDEO_AV1_COLOR_PRIMARIES_UNSPECIFIED";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_BT_470_M: return "STD_VIDEO_AV1_COLOR_PRIMARIES_BT_470_M";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_BT_470_B_G: return "STD_VIDEO_AV1_COLOR_PRIMARIES_BT_470_B_G";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_BT_601: return "STD_VIDEO_AV1_COLOR_PRIMARIES_BT_601";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_240: return "STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_240";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_GENERIC_FILM: return "STD_VIDEO_AV1_COLOR_PRIMARIES_GENERIC_FILM";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_BT_2020: return "STD_VIDEO_AV1_COLOR_PRIMARIES_BT_2020";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_XYZ: return "STD_VIDEO_AV1_COLOR_PRIMARIES_XYZ";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_431: return "STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_431";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_432: return "STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_432";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_EBU_3213: return "STD_VIDEO_AV1_COLOR_PRIMARIES_EBU_3213";
        case STD_VIDEO_AV1_COLOR_PRIMARIES_INVALID: return "STD_VIDEO_AV1_COLOR_PRIMARIES_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1ColorPrimaries_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1ColorPrimaries(Printer &p, std::string name, StdVideoAV1ColorPrimaries value) {
    p.PrintKeyString(name, StdVideoAV1ColorPrimariesString(value));
}
std::string StdVideoAV1TransferCharacteristicsString(StdVideoAV1TransferCharacteristics value) {
    switch (value) {
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_RESERVED_0: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_RESERVED_0";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_709: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_709";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_UNSPECIFIED: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_UNSPECIFIED";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_RESERVED_3: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_RESERVED_3";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_470_M: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_470_M";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_470_B_G: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_470_B_G";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_601: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_601";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_240: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_240";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LINEAR: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LINEAR";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LOG_100: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LOG_100";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LOG_100_SQRT10: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LOG_100_SQRT10";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_IEC_61966: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_IEC_61966";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_1361: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_1361";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SRGB: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SRGB";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_2020_10_BIT: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_2020_10_BIT";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_2020_12_BIT: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_2020_12_BIT";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_2084: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_2084";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_428: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_428";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_HLG: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_HLG";
        case STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_INVALID: return "STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1TransferCharacteristics_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1TransferCharacteristics(Printer &p, std::string name, StdVideoAV1TransferCharacteristics value) {
    p.PrintKeyString(name, StdVideoAV1TransferCharacteristicsString(value));
}
std::string StdVideoAV1MatrixCoefficientsString(StdVideoAV1MatrixCoefficients value) {
    switch (value) {
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_IDENTITY: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_IDENTITY";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_709: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_709";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_UNSPECIFIED: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_UNSPECIFIED";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_RESERVED_3: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_RESERVED_3";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_FCC: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_FCC";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_470_B_G: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_470_B_G";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_601: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_601";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_240: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_240";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_YCGCO: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_YCGCO";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_2020_NCL: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_2020_NCL";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_2020_CL: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_2020_CL";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_2085: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_2085";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_CHROMAT_NCL: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_CHROMAT_NCL";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_CHROMAT_CL: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_CHROMAT_CL";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_ICTCP: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_ICTCP";
        case STD_VIDEO_AV1_MATRIX_COEFFICIENTS_INVALID: return "STD_VIDEO_AV1_MATRIX_COEFFICIENTS_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1MatrixCoefficients_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1MatrixCoefficients(Printer &p, std::string name, StdVideoAV1MatrixCoefficients value) {
    p.PrintKeyString(name, StdVideoAV1MatrixCoefficientsString(value));
}
std::string StdVideoAV1ChromaSamplePositionString(StdVideoAV1ChromaSamplePosition value) {
    switch (value) {
        case STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_UNKNOWN: return "STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_UNKNOWN";
        case STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_VERTICAL: return "STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_VERTICAL";
        case STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_COLOCATED: return "STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_COLOCATED";
        case STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_RESERVED: return "STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_RESERVED";
        case STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_INVALID: return "STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_INVALID";
        default: return std::string("UNKNOWN_StdVideoAV1ChromaSamplePosition_value") + std::to_string(value);
    }
}
void DumpStdVideoAV1ChromaSamplePosition(Printer &p, std::string name, StdVideoAV1ChromaSamplePosition value) {
    p.PrintKeyString(name, StdVideoAV1ChromaSamplePositionString(value));
}
std::string VkColorSpaceKHRString(VkColorSpaceKHR value) {
    switch (value) {
        case (VK_COLOR_SPACE_SRGB_NONLINEAR_KHR): return "COLOR_SPACE_SRGB_NONLINEAR_KHR";
        case (VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT): return "COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT";
        case (VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT): return "COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
        case (VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT): return "COLOR_SPACE_DISPLAY_P3_LINEAR_EXT";
        case (VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT): return "COLOR_SPACE_DCI_P3_NONLINEAR_EXT";
        case (VK_COLOR_SPACE_BT709_LINEAR_EXT): return "COLOR_SPACE_BT709_LINEAR_EXT";
        case (VK_COLOR_SPACE_BT709_NONLINEAR_EXT): return "COLOR_SPACE_BT709_NONLINEAR_EXT";
        case (VK_COLOR_SPACE_BT2020_LINEAR_EXT): return "COLOR_SPACE_BT2020_LINEAR_EXT";
        case (VK_COLOR_SPACE_HDR10_ST2084_EXT): return "COLOR_SPACE_HDR10_ST2084_EXT";
        case (VK_COLOR_SPACE_DOLBYVISION_EXT): return "COLOR_SPACE_DOLBYVISION_EXT";
        case (VK_COLOR_SPACE_HDR10_HLG_EXT): return "COLOR_SPACE_HDR10_HLG_EXT";
        case (VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT): return "COLOR_SPACE_ADOBERGB_LINEAR_EXT";
        case (VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT): return "COLOR_SPACE_ADOBERGB_NONLINEAR_EXT";
        case (VK_COLOR_SPACE_PASS_THROUGH_EXT): return "COLOR_SPACE_PASS_THROUGH_EXT";
        case (VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT): return "COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT";
        case (VK_COLOR_SPACE_DISPLAY_NATIVE_AMD): return "COLOR_SPACE_DISPLAY_NATIVE_AMD";
        default: return std::string("UNKNOWN_VkColorSpaceKHR_value") + std::to_string(value);
    }
}
void DumpVkColorSpaceKHR(Printer &p, std::string name, VkColorSpaceKHR value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkColorSpaceKHRString(value));
    else
        p.PrintKeyString(name, VkColorSpaceKHRString(value));
}
std::string VkComponentSwizzleString(VkComponentSwizzle value) {
    switch (value) {
        case (VK_COMPONENT_SWIZZLE_IDENTITY): return "COMPONENT_SWIZZLE_IDENTITY";
        case (VK_COMPONENT_SWIZZLE_ZERO): return "COMPONENT_SWIZZLE_ZERO";
        case (VK_COMPONENT_SWIZZLE_ONE): return "COMPONENT_SWIZZLE_ONE";
        case (VK_COMPONENT_SWIZZLE_R): return "COMPONENT_SWIZZLE_R";
        case (VK_COMPONENT_SWIZZLE_G): return "COMPONENT_SWIZZLE_G";
        case (VK_COMPONENT_SWIZZLE_B): return "COMPONENT_SWIZZLE_B";
        case (VK_COMPONENT_SWIZZLE_A): return "COMPONENT_SWIZZLE_A";
        default: return std::string("UNKNOWN_VkComponentSwizzle_value") + std::to_string(value);
    }
}
void DumpVkComponentSwizzle(Printer &p, std::string name, VkComponentSwizzle value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkComponentSwizzleString(value));
    else
        p.PrintKeyString(name, VkComponentSwizzleString(value));
}
std::string VkDriverIdString(VkDriverId value) {
    switch (value) {
        case (VK_DRIVER_ID_AMD_PROPRIETARY): return "DRIVER_ID_AMD_PROPRIETARY";
        case (VK_DRIVER_ID_AMD_OPEN_SOURCE): return "DRIVER_ID_AMD_OPEN_SOURCE";
        case (VK_DRIVER_ID_MESA_RADV): return "DRIVER_ID_MESA_RADV";
        case (VK_DRIVER_ID_NVIDIA_PROPRIETARY): return "DRIVER_ID_NVIDIA_PROPRIETARY";
        case (VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS): return "DRIVER_ID_INTEL_PROPRIETARY_WINDOWS";
        case (VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA): return "DRIVER_ID_INTEL_OPEN_SOURCE_MESA";
        case (VK_DRIVER_ID_IMAGINATION_PROPRIETARY): return "DRIVER_ID_IMAGINATION_PROPRIETARY";
        case (VK_DRIVER_ID_QUALCOMM_PROPRIETARY): return "DRIVER_ID_QUALCOMM_PROPRIETARY";
        case (VK_DRIVER_ID_ARM_PROPRIETARY): return "DRIVER_ID_ARM_PROPRIETARY";
        case (VK_DRIVER_ID_GOOGLE_SWIFTSHADER): return "DRIVER_ID_GOOGLE_SWIFTSHADER";
        case (VK_DRIVER_ID_GGP_PROPRIETARY): return "DRIVER_ID_GGP_PROPRIETARY";
        case (VK_DRIVER_ID_BROADCOM_PROPRIETARY): return "DRIVER_ID_BROADCOM_PROPRIETARY";
        case (VK_DRIVER_ID_MESA_LLVMPIPE): return "DRIVER_ID_MESA_LLVMPIPE";
        case (VK_DRIVER_ID_MOLTENVK): return "DRIVER_ID_MOLTENVK";
        case (VK_DRIVER_ID_COREAVI_PROPRIETARY): return "DRIVER_ID_COREAVI_PROPRIETARY";
        case (VK_DRIVER_ID_JUICE_PROPRIETARY): return "DRIVER_ID_JUICE_PROPRIETARY";
        case (VK_DRIVER_ID_VERISILICON_PROPRIETARY): return "DRIVER_ID_VERISILICON_PROPRIETARY";
        case (VK_DRIVER_ID_MESA_TURNIP): return "DRIVER_ID_MESA_TURNIP";
        case (VK_DRIVER_ID_MESA_V3DV): return "DRIVER_ID_MESA_V3DV";
        case (VK_DRIVER_ID_MESA_PANVK): return "DRIVER_ID_MESA_PANVK";
        case (VK_DRIVER_ID_SAMSUNG_PROPRIETARY): return "DRIVER_ID_SAMSUNG_PROPRIETARY";
        case (VK_DRIVER_ID_MESA_VENUS): return "DRIVER_ID_MESA_VENUS";
        case (VK_DRIVER_ID_MESA_DOZEN): return "DRIVER_ID_MESA_DOZEN";
        case (VK_DRIVER_ID_MESA_NVK): return "DRIVER_ID_MESA_NVK";
        case (VK_DRIVER_ID_IMAGINATION_OPEN_SOURCE_MESA): return "DRIVER_ID_IMAGINATION_OPEN_SOURCE_MESA";
        case (VK_DRIVER_ID_MESA_HONEYKRISP): return "DRIVER_ID_MESA_HONEYKRISP";
        case (VK_DRIVER_ID_VULKAN_SC_EMULATION_ON_VULKAN): return "DRIVER_ID_VULKAN_SC_EMULATION_ON_VULKAN";
        default: return std::string("UNKNOWN_VkDriverId_value") + std::to_string(value);
    }
}
void DumpVkDriverId(Printer &p, std::string name, VkDriverId value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkDriverIdString(value));
    else
        p.PrintKeyString(name, VkDriverIdString(value));
}
std::string VkFormatString(VkFormat value) {
    switch (value) {
        case (VK_FORMAT_UNDEFINED): return "FORMAT_UNDEFINED";
        case (VK_FORMAT_R4G4_UNORM_PACK8): return "FORMAT_R4G4_UNORM_PACK8";
        case (VK_FORMAT_R4G4B4A4_UNORM_PACK16): return "FORMAT_R4G4B4A4_UNORM_PACK16";
        case (VK_FORMAT_B4G4R4A4_UNORM_PACK16): return "FORMAT_B4G4R4A4_UNORM_PACK16";
        case (VK_FORMAT_R5G6B5_UNORM_PACK16): return "FORMAT_R5G6B5_UNORM_PACK16";
        case (VK_FORMAT_B5G6R5_UNORM_PACK16): return "FORMAT_B5G6R5_UNORM_PACK16";
        case (VK_FORMAT_R5G5B5A1_UNORM_PACK16): return "FORMAT_R5G5B5A1_UNORM_PACK16";
        case (VK_FORMAT_B5G5R5A1_UNORM_PACK16): return "FORMAT_B5G5R5A1_UNORM_PACK16";
        case (VK_FORMAT_A1R5G5B5_UNORM_PACK16): return "FORMAT_A1R5G5B5_UNORM_PACK16";
        case (VK_FORMAT_R8_UNORM): return "FORMAT_R8_UNORM";
        case (VK_FORMAT_R8_SNORM): return "FORMAT_R8_SNORM";
        case (VK_FORMAT_R8_USCALED): return "FORMAT_R8_USCALED";
        case (VK_FORMAT_R8_SSCALED): return "FORMAT_R8_SSCALED";
        case (VK_FORMAT_R8_UINT): return "FORMAT_R8_UINT";
        case (VK_FORMAT_R8_SINT): return "FORMAT_R8_SINT";
        case (VK_FORMAT_R8_SRGB): return "FORMAT_R8_SRGB";
        case (VK_FORMAT_R8G8_UNORM): return "FORMAT_R8G8_UNORM";
        case (VK_FORMAT_R8G8_SNORM): return "FORMAT_R8G8_SNORM";
        case (VK_FORMAT_R8G8_USCALED): return "FORMAT_R8G8_USCALED";
        case (VK_FORMAT_R8G8_SSCALED): return "FORMAT_R8G8_SSCALED";
        case (VK_FORMAT_R8G8_UINT): return "FORMAT_R8G8_UINT";
        case (VK_FORMAT_R8G8_SINT): return "FORMAT_R8G8_SINT";
        case (VK_FORMAT_R8G8_SRGB): return "FORMAT_R8G8_SRGB";
        case (VK_FORMAT_R8G8B8_UNORM): return "FORMAT_R8G8B8_UNORM";
        case (VK_FORMAT_R8G8B8_SNORM): return "FORMAT_R8G8B8_SNORM";
        case (VK_FORMAT_R8G8B8_USCALED): return "FORMAT_R8G8B8_USCALED";
        case (VK_FORMAT_R8G8B8_SSCALED): return "FORMAT_R8G8B8_SSCALED";
        case (VK_FORMAT_R8G8B8_UINT): return "FORMAT_R8G8B8_UINT";
        case (VK_FORMAT_R8G8B8_SINT): return "FORMAT_R8G8B8_SINT";
        case (VK_FORMAT_R8G8B8_SRGB): return "FORMAT_R8G8B8_SRGB";
        case (VK_FORMAT_B8G8R8_UNORM): return "FORMAT_B8G8R8_UNORM";
        case (VK_FORMAT_B8G8R8_SNORM): return "FORMAT_B8G8R8_SNORM";
        case (VK_FORMAT_B8G8R8_USCALED): return "FORMAT_B8G8R8_USCALED";
        case (VK_FORMAT_B8G8R8_SSCALED): return "FORMAT_B8G8R8_SSCALED";
        case (VK_FORMAT_B8G8R8_UINT): return "FORMAT_B8G8R8_UINT";
        case (VK_FORMAT_B8G8R8_SINT): return "FORMAT_B8G8R8_SINT";
        case (VK_FORMAT_B8G8R8_SRGB): return "FORMAT_B8G8R8_SRGB";
        case (VK_FORMAT_R8G8B8A8_UNORM): return "FORMAT_R8G8B8A8_UNORM";
        case (VK_FORMAT_R8G8B8A8_SNORM): return "FORMAT_R8G8B8A8_SNORM";
        case (VK_FORMAT_R8G8B8A8_USCALED): return "FORMAT_R8G8B8A8_USCALED";
        case (VK_FORMAT_R8G8B8A8_SSCALED): return "FORMAT_R8G8B8A8_SSCALED";
        case (VK_FORMAT_R8G8B8A8_UINT): return "FORMAT_R8G8B8A8_UINT";
        case (VK_FORMAT_R8G8B8A8_SINT): return "FORMAT_R8G8B8A8_SINT";
        case (VK_FORMAT_R8G8B8A8_SRGB): return "FORMAT_R8G8B8A8_SRGB";
        case (VK_FORMAT_B8G8R8A8_UNORM): return "FORMAT_B8G8R8A8_UNORM";
        case (VK_FORMAT_B8G8R8A8_SNORM): return "FORMAT_B8G8R8A8_SNORM";
        case (VK_FORMAT_B8G8R8A8_USCALED): return "FORMAT_B8G8R8A8_USCALED";
        case (VK_FORMAT_B8G8R8A8_SSCALED): return "FORMAT_B8G8R8A8_SSCALED";
        case (VK_FORMAT_B8G8R8A8_UINT): return "FORMAT_B8G8R8A8_UINT";
        case (VK_FORMAT_B8G8R8A8_SINT): return "FORMAT_B8G8R8A8_SINT";
        case (VK_FORMAT_B8G8R8A8_SRGB): return "FORMAT_B8G8R8A8_SRGB";
        case (VK_FORMAT_A8B8G8R8_UNORM_PACK32): return "FORMAT_A8B8G8R8_UNORM_PACK32";
        case (VK_FORMAT_A8B8G8R8_SNORM_PACK32): return "FORMAT_A8B8G8R8_SNORM_PACK32";
        case (VK_FORMAT_A8B8G8R8_USCALED_PACK32): return "FORMAT_A8B8G8R8_USCALED_PACK32";
        case (VK_FORMAT_A8B8G8R8_SSCALED_PACK32): return "FORMAT_A8B8G8R8_SSCALED_PACK32";
        case (VK_FORMAT_A8B8G8R8_UINT_PACK32): return "FORMAT_A8B8G8R8_UINT_PACK32";
        case (VK_FORMAT_A8B8G8R8_SINT_PACK32): return "FORMAT_A8B8G8R8_SINT_PACK32";
        case (VK_FORMAT_A8B8G8R8_SRGB_PACK32): return "FORMAT_A8B8G8R8_SRGB_PACK32";
        case (VK_FORMAT_A2R10G10B10_UNORM_PACK32): return "FORMAT_A2R10G10B10_UNORM_PACK32";
        case (VK_FORMAT_A2R10G10B10_SNORM_PACK32): return "FORMAT_A2R10G10B10_SNORM_PACK32";
        case (VK_FORMAT_A2R10G10B10_USCALED_PACK32): return "FORMAT_A2R10G10B10_USCALED_PACK32";
        case (VK_FORMAT_A2R10G10B10_SSCALED_PACK32): return "FORMAT_A2R10G10B10_SSCALED_PACK32";
        case (VK_FORMAT_A2R10G10B10_UINT_PACK32): return "FORMAT_A2R10G10B10_UINT_PACK32";
        case (VK_FORMAT_A2R10G10B10_SINT_PACK32): return "FORMAT_A2R10G10B10_SINT_PACK32";
        case (VK_FORMAT_A2B10G10R10_UNORM_PACK32): return "FORMAT_A2B10G10R10_UNORM_PACK32";
        case (VK_FORMAT_A2B10G10R10_SNORM_PACK32): return "FORMAT_A2B10G10R10_SNORM_PACK32";
        case (VK_FORMAT_A2B10G10R10_USCALED_PACK32): return "FORMAT_A2B10G10R10_USCALED_PACK32";
        case (VK_FORMAT_A2B10G10R10_SSCALED_PACK32): return "FORMAT_A2B10G10R10_SSCALED_PACK32";
        case (VK_FORMAT_A2B10G10R10_UINT_PACK32): return "FORMAT_A2B10G10R10_UINT_PACK32";
        case (VK_FORMAT_A2B10G10R10_SINT_PACK32): return "FORMAT_A2B10G10R10_SINT_PACK32";
        case (VK_FORMAT_R16_UNORM): return "FORMAT_R16_UNORM";
        case (VK_FORMAT_R16_SNORM): return "FORMAT_R16_SNORM";
        case (VK_FORMAT_R16_USCALED): return "FORMAT_R16_USCALED";
        case (VK_FORMAT_R16_SSCALED): return "FORMAT_R16_SSCALED";
        case (VK_FORMAT_R16_UINT): return "FORMAT_R16_UINT";
        case (VK_FORMAT_R16_SINT): return "FORMAT_R16_SINT";
        case (VK_FORMAT_R16_SFLOAT): return "FORMAT_R16_SFLOAT";
        case (VK_FORMAT_R16G16_UNORM): return "FORMAT_R16G16_UNORM";
        case (VK_FORMAT_R16G16_SNORM): return "FORMAT_R16G16_SNORM";
        case (VK_FORMAT_R16G16_USCALED): return "FORMAT_R16G16_USCALED";
        case (VK_FORMAT_R16G16_SSCALED): return "FORMAT_R16G16_SSCALED";
        case (VK_FORMAT_R16G16_UINT): return "FORMAT_R16G16_UINT";
        case (VK_FORMAT_R16G16_SINT): return "FORMAT_R16G16_SINT";
        case (VK_FORMAT_R16G16_SFLOAT): return "FORMAT_R16G16_SFLOAT";
        case (VK_FORMAT_R16G16B16_UNORM): return "FORMAT_R16G16B16_UNORM";
        case (VK_FORMAT_R16G16B16_SNORM): return "FORMAT_R16G16B16_SNORM";
        case (VK_FORMAT_R16G16B16_USCALED): return "FORMAT_R16G16B16_USCALED";
        case (VK_FORMAT_R16G16B16_SSCALED): return "FORMAT_R16G16B16_SSCALED";
        case (VK_FORMAT_R16G16B16_UINT): return "FORMAT_R16G16B16_UINT";
        case (VK_FORMAT_R16G16B16_SINT): return "FORMAT_R16G16B16_SINT";
        case (VK_FORMAT_R16G16B16_SFLOAT): return "FORMAT_R16G16B16_SFLOAT";
        case (VK_FORMAT_R16G16B16A16_UNORM): return "FORMAT_R16G16B16A16_UNORM";
        case (VK_FORMAT_R16G16B16A16_SNORM): return "FORMAT_R16G16B16A16_SNORM";
        case (VK_FORMAT_R16G16B16A16_USCALED): return "FORMAT_R16G16B16A16_USCALED";
        case (VK_FORMAT_R16G16B16A16_SSCALED): return "FORMAT_R16G16B16A16_SSCALED";
        case (VK_FORMAT_R16G16B16A16_UINT): return "FORMAT_R16G16B16A16_UINT";
        case (VK_FORMAT_R16G16B16A16_SINT): return "FORMAT_R16G16B16A16_SINT";
        case (VK_FORMAT_R16G16B16A16_SFLOAT): return "FORMAT_R16G16B16A16_SFLOAT";
        case (VK_FORMAT_R32_UINT): return "FORMAT_R32_UINT";
        case (VK_FORMAT_R32_SINT): return "FORMAT_R32_SINT";
        case (VK_FORMAT_R32_SFLOAT): return "FORMAT_R32_SFLOAT";
        case (VK_FORMAT_R32G32_UINT): return "FORMAT_R32G32_UINT";
        case (VK_FORMAT_R32G32_SINT): return "FORMAT_R32G32_SINT";
        case (VK_FORMAT_R32G32_SFLOAT): return "FORMAT_R32G32_SFLOAT";
        case (VK_FORMAT_R32G32B32_UINT): return "FORMAT_R32G32B32_UINT";
        case (VK_FORMAT_R32G32B32_SINT): return "FORMAT_R32G32B32_SINT";
        case (VK_FORMAT_R32G32B32_SFLOAT): return "FORMAT_R32G32B32_SFLOAT";
        case (VK_FORMAT_R32G32B32A32_UINT): return "FORMAT_R32G32B32A32_UINT";
        case (VK_FORMAT_R32G32B32A32_SINT): return "FORMAT_R32G32B32A32_SINT";
        case (VK_FORMAT_R32G32B32A32_SFLOAT): return "FORMAT_R32G32B32A32_SFLOAT";
        case (VK_FORMAT_R64_UINT): return "FORMAT_R64_UINT";
        case (VK_FORMAT_R64_SINT): return "FORMAT_R64_SINT";
        case (VK_FORMAT_R64_SFLOAT): return "FORMAT_R64_SFLOAT";
        case (VK_FORMAT_R64G64_UINT): return "FORMAT_R64G64_UINT";
        case (VK_FORMAT_R64G64_SINT): return "FORMAT_R64G64_SINT";
        case (VK_FORMAT_R64G64_SFLOAT): return "FORMAT_R64G64_SFLOAT";
        case (VK_FORMAT_R64G64B64_UINT): return "FORMAT_R64G64B64_UINT";
        case (VK_FORMAT_R64G64B64_SINT): return "FORMAT_R64G64B64_SINT";
        case (VK_FORMAT_R64G64B64_SFLOAT): return "FORMAT_R64G64B64_SFLOAT";
        case (VK_FORMAT_R64G64B64A64_UINT): return "FORMAT_R64G64B64A64_UINT";
        case (VK_FORMAT_R64G64B64A64_SINT): return "FORMAT_R64G64B64A64_SINT";
        case (VK_FORMAT_R64G64B64A64_SFLOAT): return "FORMAT_R64G64B64A64_SFLOAT";
        case (VK_FORMAT_B10G11R11_UFLOAT_PACK32): return "FORMAT_B10G11R11_UFLOAT_PACK32";
        case (VK_FORMAT_E5B9G9R9_UFLOAT_PACK32): return "FORMAT_E5B9G9R9_UFLOAT_PACK32";
        case (VK_FORMAT_D16_UNORM): return "FORMAT_D16_UNORM";
        case (VK_FORMAT_X8_D24_UNORM_PACK32): return "FORMAT_X8_D24_UNORM_PACK32";
        case (VK_FORMAT_D32_SFLOAT): return "FORMAT_D32_SFLOAT";
        case (VK_FORMAT_S8_UINT): return "FORMAT_S8_UINT";
        case (VK_FORMAT_D16_UNORM_S8_UINT): return "FORMAT_D16_UNORM_S8_UINT";
        case (VK_FORMAT_D24_UNORM_S8_UINT): return "FORMAT_D24_UNORM_S8_UINT";
        case (VK_FORMAT_D32_SFLOAT_S8_UINT): return "FORMAT_D32_SFLOAT_S8_UINT";
        case (VK_FORMAT_BC1_RGB_UNORM_BLOCK): return "FORMAT_BC1_RGB_UNORM_BLOCK";
        case (VK_FORMAT_BC1_RGB_SRGB_BLOCK): return "FORMAT_BC1_RGB_SRGB_BLOCK";
        case (VK_FORMAT_BC1_RGBA_UNORM_BLOCK): return "FORMAT_BC1_RGBA_UNORM_BLOCK";
        case (VK_FORMAT_BC1_RGBA_SRGB_BLOCK): return "FORMAT_BC1_RGBA_SRGB_BLOCK";
        case (VK_FORMAT_BC2_UNORM_BLOCK): return "FORMAT_BC2_UNORM_BLOCK";
        case (VK_FORMAT_BC2_SRGB_BLOCK): return "FORMAT_BC2_SRGB_BLOCK";
        case (VK_FORMAT_BC3_UNORM_BLOCK): return "FORMAT_BC3_UNORM_BLOCK";
        case (VK_FORMAT_BC3_SRGB_BLOCK): return "FORMAT_BC3_SRGB_BLOCK";
        case (VK_FORMAT_BC4_UNORM_BLOCK): return "FORMAT_BC4_UNORM_BLOCK";
        case (VK_FORMAT_BC4_SNORM_BLOCK): return "FORMAT_BC4_SNORM_BLOCK";
        case (VK_FORMAT_BC5_UNORM_BLOCK): return "FORMAT_BC5_UNORM_BLOCK";
        case (VK_FORMAT_BC5_SNORM_BLOCK): return "FORMAT_BC5_SNORM_BLOCK";
        case (VK_FORMAT_BC6H_UFLOAT_BLOCK): return "FORMAT_BC6H_UFLOAT_BLOCK";
        case (VK_FORMAT_BC6H_SFLOAT_BLOCK): return "FORMAT_BC6H_SFLOAT_BLOCK";
        case (VK_FORMAT_BC7_UNORM_BLOCK): return "FORMAT_BC7_UNORM_BLOCK";
        case (VK_FORMAT_BC7_SRGB_BLOCK): return "FORMAT_BC7_SRGB_BLOCK";
        case (VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK): return "FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
        case (VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK): return "FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
        case (VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK): return "FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
        case (VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK): return "FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
        case (VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK): return "FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
        case (VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK): return "FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
        case (VK_FORMAT_EAC_R11_UNORM_BLOCK): return "FORMAT_EAC_R11_UNORM_BLOCK";
        case (VK_FORMAT_EAC_R11_SNORM_BLOCK): return "FORMAT_EAC_R11_SNORM_BLOCK";
        case (VK_FORMAT_EAC_R11G11_UNORM_BLOCK): return "FORMAT_EAC_R11G11_UNORM_BLOCK";
        case (VK_FORMAT_EAC_R11G11_SNORM_BLOCK): return "FORMAT_EAC_R11G11_SNORM_BLOCK";
        case (VK_FORMAT_ASTC_4x4_UNORM_BLOCK): return "FORMAT_ASTC_4x4_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_4x4_SRGB_BLOCK): return "FORMAT_ASTC_4x4_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_5x4_UNORM_BLOCK): return "FORMAT_ASTC_5x4_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_5x4_SRGB_BLOCK): return "FORMAT_ASTC_5x4_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_5x5_UNORM_BLOCK): return "FORMAT_ASTC_5x5_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_5x5_SRGB_BLOCK): return "FORMAT_ASTC_5x5_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_6x5_UNORM_BLOCK): return "FORMAT_ASTC_6x5_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_6x5_SRGB_BLOCK): return "FORMAT_ASTC_6x5_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_6x6_UNORM_BLOCK): return "FORMAT_ASTC_6x6_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_6x6_SRGB_BLOCK): return "FORMAT_ASTC_6x6_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_8x5_UNORM_BLOCK): return "FORMAT_ASTC_8x5_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_8x5_SRGB_BLOCK): return "FORMAT_ASTC_8x5_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_8x6_UNORM_BLOCK): return "FORMAT_ASTC_8x6_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_8x6_SRGB_BLOCK): return "FORMAT_ASTC_8x6_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_8x8_UNORM_BLOCK): return "FORMAT_ASTC_8x8_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_8x8_SRGB_BLOCK): return "FORMAT_ASTC_8x8_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_10x5_UNORM_BLOCK): return "FORMAT_ASTC_10x5_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_10x5_SRGB_BLOCK): return "FORMAT_ASTC_10x5_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_10x6_UNORM_BLOCK): return "FORMAT_ASTC_10x6_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_10x6_SRGB_BLOCK): return "FORMAT_ASTC_10x6_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_10x8_UNORM_BLOCK): return "FORMAT_ASTC_10x8_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_10x8_SRGB_BLOCK): return "FORMAT_ASTC_10x8_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_10x10_UNORM_BLOCK): return "FORMAT_ASTC_10x10_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_10x10_SRGB_BLOCK): return "FORMAT_ASTC_10x10_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_12x10_UNORM_BLOCK): return "FORMAT_ASTC_12x10_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_12x10_SRGB_BLOCK): return "FORMAT_ASTC_12x10_SRGB_BLOCK";
        case (VK_FORMAT_ASTC_12x12_UNORM_BLOCK): return "FORMAT_ASTC_12x12_UNORM_BLOCK";
        case (VK_FORMAT_ASTC_12x12_SRGB_BLOCK): return "FORMAT_ASTC_12x12_SRGB_BLOCK";
        case (VK_FORMAT_G8B8G8R8_422_UNORM): return "FORMAT_G8B8G8R8_422_UNORM";
        case (VK_FORMAT_B8G8R8G8_422_UNORM): return "FORMAT_B8G8R8G8_422_UNORM";
        case (VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM): return "FORMAT_G8_B8_R8_3PLANE_420_UNORM";
        case (VK_FORMAT_G8_B8R8_2PLANE_420_UNORM): return "FORMAT_G8_B8R8_2PLANE_420_UNORM";
        case (VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM): return "FORMAT_G8_B8_R8_3PLANE_422_UNORM";
        case (VK_FORMAT_G8_B8R8_2PLANE_422_UNORM): return "FORMAT_G8_B8R8_2PLANE_422_UNORM";
        case (VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM): return "FORMAT_G8_B8_R8_3PLANE_444_UNORM";
        case (VK_FORMAT_R10X6_UNORM_PACK16): return "FORMAT_R10X6_UNORM_PACK16";
        case (VK_FORMAT_R10X6G10X6_UNORM_2PACK16): return "FORMAT_R10X6G10X6_UNORM_2PACK16";
        case (VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16): return "FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16";
        case (VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16): return "FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
        case (VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16): return "FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
        case (VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16): return "FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
        case (VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16): return "FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
        case (VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16): return "FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
        case (VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16): return "FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
        case (VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16): return "FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
        case (VK_FORMAT_R12X4_UNORM_PACK16): return "FORMAT_R12X4_UNORM_PACK16";
        case (VK_FORMAT_R12X4G12X4_UNORM_2PACK16): return "FORMAT_R12X4G12X4_UNORM_2PACK16";
        case (VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16): return "FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16";
        case (VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16): return "FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
        case (VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16): return "FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
        case (VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16): return "FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
        case (VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16): return "FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
        case (VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16): return "FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
        case (VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16): return "FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
        case (VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16): return "FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
        case (VK_FORMAT_G16B16G16R16_422_UNORM): return "FORMAT_G16B16G16R16_422_UNORM";
        case (VK_FORMAT_B16G16R16G16_422_UNORM): return "FORMAT_B16G16R16G16_422_UNORM";
        case (VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM): return "FORMAT_G16_B16_R16_3PLANE_420_UNORM";
        case (VK_FORMAT_G16_B16R16_2PLANE_420_UNORM): return "FORMAT_G16_B16R16_2PLANE_420_UNORM";
        case (VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM): return "FORMAT_G16_B16_R16_3PLANE_422_UNORM";
        case (VK_FORMAT_G16_B16R16_2PLANE_422_UNORM): return "FORMAT_G16_B16R16_2PLANE_422_UNORM";
        case (VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM): return "FORMAT_G16_B16_R16_3PLANE_444_UNORM";
        case (VK_FORMAT_G8_B8R8_2PLANE_444_UNORM): return "FORMAT_G8_B8R8_2PLANE_444_UNORM";
        case (VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16): return "FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16";
        case (VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16): return "FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16";
        case (VK_FORMAT_G16_B16R16_2PLANE_444_UNORM): return "FORMAT_G16_B16R16_2PLANE_444_UNORM";
        case (VK_FORMAT_A4R4G4B4_UNORM_PACK16): return "FORMAT_A4R4G4B4_UNORM_PACK16";
        case (VK_FORMAT_A4B4G4R4_UNORM_PACK16): return "FORMAT_A4B4G4R4_UNORM_PACK16";
        case (VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK): return "FORMAT_ASTC_4x4_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK): return "FORMAT_ASTC_5x4_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK): return "FORMAT_ASTC_5x5_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK): return "FORMAT_ASTC_6x5_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK): return "FORMAT_ASTC_6x6_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK): return "FORMAT_ASTC_8x5_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK): return "FORMAT_ASTC_8x6_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK): return "FORMAT_ASTC_8x8_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK): return "FORMAT_ASTC_10x5_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK): return "FORMAT_ASTC_10x6_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK): return "FORMAT_ASTC_10x8_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK): return "FORMAT_ASTC_10x10_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK): return "FORMAT_ASTC_12x10_SFLOAT_BLOCK";
        case (VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK): return "FORMAT_ASTC_12x12_SFLOAT_BLOCK";
        case (VK_FORMAT_A1B5G5R5_UNORM_PACK16): return "FORMAT_A1B5G5R5_UNORM_PACK16";
        case (VK_FORMAT_A8_UNORM): return "FORMAT_A8_UNORM";
        case (VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG): return "FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
        case (VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG): return "FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
        case (VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG): return "FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
        case (VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG): return "FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
        case (VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG): return "FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
        case (VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG): return "FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
        case (VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG): return "FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
        case (VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG): return "FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
        case (VK_FORMAT_R16G16_SFIXED5_NV): return "FORMAT_R16G16_SFIXED5_NV";
        default: return std::string("UNKNOWN_VkFormat_value") + std::to_string(value);
    }
}
void DumpVkFormat(Printer &p, std::string name, VkFormat value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkFormatString(value));
    else
        p.PrintKeyString(name, VkFormatString(value));
}
std::string VkImageLayoutString(VkImageLayout value) {
    switch (value) {
        case (VK_IMAGE_LAYOUT_UNDEFINED): return "IMAGE_LAYOUT_UNDEFINED";
        case (VK_IMAGE_LAYOUT_GENERAL): return "IMAGE_LAYOUT_GENERAL";
        case (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL): return "IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL";
        case (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL): return "IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
        case (VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL): return "IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL";
        case (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL): return "IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL";
        case (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL): return "IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL";
        case (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL): return "IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL";
        case (VK_IMAGE_LAYOUT_PREINITIALIZED): return "IMAGE_LAYOUT_PREINITIALIZED";
        case (VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL): return "IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
        case (VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL): return "IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
        case (VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL): return "IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL";
        case (VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL): return "IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL";
        case (VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL): return "IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL";
        case (VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL): return "IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL";
        case (VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL): return "IMAGE_LAYOUT_READ_ONLY_OPTIMAL";
        case (VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL): return "IMAGE_LAYOUT_ATTACHMENT_OPTIMAL";
        case (VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ): return "IMAGE_LAYOUT_RENDERING_LOCAL_READ";
        case (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR): return "IMAGE_LAYOUT_PRESENT_SRC_KHR";
        case (VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR): return "IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR";
        case (VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR): return "IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR";
        case (VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR): return "IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR";
        case (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR): return "IMAGE_LAYOUT_SHARED_PRESENT_KHR";
        case (VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT): return "IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT";
        case (VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR): return "IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR";
        case (VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR): return "IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR";
        case (VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR): return "IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR";
        case (VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR): return "IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR";
        case (VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT): return "IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT";
        case (VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR): return "IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR";
        default: return std::string("UNKNOWN_VkImageLayout_value") + std::to_string(value);
    }
}
void DumpVkImageLayout(Printer &p, std::string name, VkImageLayout value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkImageLayoutString(value));
    else
        p.PrintKeyString(name, VkImageLayoutString(value));
}
std::string VkImageTilingString(VkImageTiling value) {
    switch (value) {
        case (VK_IMAGE_TILING_OPTIMAL): return "IMAGE_TILING_OPTIMAL";
        case (VK_IMAGE_TILING_LINEAR): return "IMAGE_TILING_LINEAR";
        case (VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT): return "IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT";
        default: return std::string("UNKNOWN_VkImageTiling_value") + std::to_string(value);
    }
}
void DumpVkImageTiling(Printer &p, std::string name, VkImageTiling value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkImageTilingString(value));
    else
        p.PrintKeyString(name, VkImageTilingString(value));
}
std::string VkImageTypeString(VkImageType value) {
    switch (value) {
        case (VK_IMAGE_TYPE_1D): return "IMAGE_TYPE_1D";
        case (VK_IMAGE_TYPE_2D): return "IMAGE_TYPE_2D";
        case (VK_IMAGE_TYPE_3D): return "IMAGE_TYPE_3D";
        default: return std::string("UNKNOWN_VkImageType_value") + std::to_string(value);
    }
}
void DumpVkImageType(Printer &p, std::string name, VkImageType value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkImageTypeString(value));
    else
        p.PrintKeyString(name, VkImageTypeString(value));
}
std::string VkPhysicalDeviceLayeredApiKHRString(VkPhysicalDeviceLayeredApiKHR value) {
    switch (value) {
        case (VK_PHYSICAL_DEVICE_LAYERED_API_VULKAN_KHR): return "PHYSICAL_DEVICE_LAYERED_API_VULKAN_KHR";
        case (VK_PHYSICAL_DEVICE_LAYERED_API_D3D12_KHR): return "PHYSICAL_DEVICE_LAYERED_API_D3D12_KHR";
        case (VK_PHYSICAL_DEVICE_LAYERED_API_METAL_KHR): return "PHYSICAL_DEVICE_LAYERED_API_METAL_KHR";
        case (VK_PHYSICAL_DEVICE_LAYERED_API_OPENGL_KHR): return "PHYSICAL_DEVICE_LAYERED_API_OPENGL_KHR";
        case (VK_PHYSICAL_DEVICE_LAYERED_API_OPENGLES_KHR): return "PHYSICAL_DEVICE_LAYERED_API_OPENGLES_KHR";
        default: return std::string("UNKNOWN_VkPhysicalDeviceLayeredApiKHR_value") + std::to_string(value);
    }
}
void DumpVkPhysicalDeviceLayeredApiKHR(Printer &p, std::string name, VkPhysicalDeviceLayeredApiKHR value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkPhysicalDeviceLayeredApiKHRString(value));
    else
        p.PrintKeyString(name, VkPhysicalDeviceLayeredApiKHRString(value));
}
std::string VkPhysicalDeviceTypeString(VkPhysicalDeviceType value) {
    switch (value) {
        case (VK_PHYSICAL_DEVICE_TYPE_OTHER): return "PHYSICAL_DEVICE_TYPE_OTHER";
        case (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU): return "PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
        case (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU): return "PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
        case (VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU): return "PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
        case (VK_PHYSICAL_DEVICE_TYPE_CPU): return "PHYSICAL_DEVICE_TYPE_CPU";
        default: return std::string("UNKNOWN_VkPhysicalDeviceType_value") + std::to_string(value);
    }
}
void DumpVkPhysicalDeviceType(Printer &p, std::string name, VkPhysicalDeviceType value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkPhysicalDeviceTypeString(value));
    else
        p.PrintKeyString(name, VkPhysicalDeviceTypeString(value));
}
std::string VkPipelineRobustnessBufferBehaviorString(VkPipelineRobustnessBufferBehavior value) {
    switch (value) {
        case (VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT): return "PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT";
        case (VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED): return "PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED";
        case (VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS): return "PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS";
        case (VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2): return "PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2";
        default: return std::string("UNKNOWN_VkPipelineRobustnessBufferBehavior_value") + std::to_string(value);
    }
}
void DumpVkPipelineRobustnessBufferBehavior(Printer &p, std::string name, VkPipelineRobustnessBufferBehavior value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkPipelineRobustnessBufferBehaviorString(value));
    else
        p.PrintKeyString(name, VkPipelineRobustnessBufferBehaviorString(value));
}
std::string VkPipelineRobustnessImageBehaviorString(VkPipelineRobustnessImageBehavior value) {
    switch (value) {
        case (VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT): return "PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT";
        case (VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED): return "PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED";
        case (VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS): return "PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS";
        case (VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2): return "PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2";
        default: return std::string("UNKNOWN_VkPipelineRobustnessImageBehavior_value") + std::to_string(value);
    }
}
void DumpVkPipelineRobustnessImageBehavior(Printer &p, std::string name, VkPipelineRobustnessImageBehavior value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkPipelineRobustnessImageBehaviorString(value));
    else
        p.PrintKeyString(name, VkPipelineRobustnessImageBehaviorString(value));
}
std::string VkPointClippingBehaviorString(VkPointClippingBehavior value) {
    switch (value) {
        case (VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES): return "POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES";
        case (VK_POINT_CLIPPING_BEHAVIOR_USER_CLIP_PLANES_ONLY): return "POINT_CLIPPING_BEHAVIOR_USER_CLIP_PLANES_ONLY";
        default: return std::string("UNKNOWN_VkPointClippingBehavior_value") + std::to_string(value);
    }
}
void DumpVkPointClippingBehavior(Printer &p, std::string name, VkPointClippingBehavior value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkPointClippingBehaviorString(value));
    else
        p.PrintKeyString(name, VkPointClippingBehaviorString(value));
}
std::string VkPresentModeKHRString(VkPresentModeKHR value) {
    switch (value) {
        case (VK_PRESENT_MODE_IMMEDIATE_KHR): return "PRESENT_MODE_IMMEDIATE_KHR";
        case (VK_PRESENT_MODE_MAILBOX_KHR): return "PRESENT_MODE_MAILBOX_KHR";
        case (VK_PRESENT_MODE_FIFO_KHR): return "PRESENT_MODE_FIFO_KHR";
        case (VK_PRESENT_MODE_FIFO_RELAXED_KHR): return "PRESENT_MODE_FIFO_RELAXED_KHR";
        case (VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR): return "PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
        case (VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR): return "PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
        case (VK_PRESENT_MODE_FIFO_LATEST_READY_EXT): return "PRESENT_MODE_FIFO_LATEST_READY_EXT";
        default: return std::string("UNKNOWN_VkPresentModeKHR_value") + std::to_string(value);
    }
}
void DumpVkPresentModeKHR(Printer &p, std::string name, VkPresentModeKHR value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkPresentModeKHRString(value));
    else
        p.PrintKeyString(name, VkPresentModeKHRString(value));
}
std::string VkQueueGlobalPriorityString(VkQueueGlobalPriority value) {
    switch (value) {
        case (VK_QUEUE_GLOBAL_PRIORITY_LOW): return "QUEUE_GLOBAL_PRIORITY_LOW";
        case (VK_QUEUE_GLOBAL_PRIORITY_MEDIUM): return "QUEUE_GLOBAL_PRIORITY_MEDIUM";
        case (VK_QUEUE_GLOBAL_PRIORITY_HIGH): return "QUEUE_GLOBAL_PRIORITY_HIGH";
        case (VK_QUEUE_GLOBAL_PRIORITY_REALTIME): return "QUEUE_GLOBAL_PRIORITY_REALTIME";
        default: return std::string("UNKNOWN_VkQueueGlobalPriority_value") + std::to_string(value);
    }
}
void DumpVkQueueGlobalPriority(Printer &p, std::string name, VkQueueGlobalPriority value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkQueueGlobalPriorityString(value));
    else
        p.PrintKeyString(name, VkQueueGlobalPriorityString(value));
}
std::string VkResultString(VkResult value) {
    switch (value) {
        case (VK_SUCCESS): return "SUCCESS";
        case (VK_NOT_READY): return "NOT_READY";
        case (VK_TIMEOUT): return "TIMEOUT";
        case (VK_EVENT_SET): return "EVENT_SET";
        case (VK_EVENT_RESET): return "EVENT_RESET";
        case (VK_INCOMPLETE): return "INCOMPLETE";
        case (VK_ERROR_OUT_OF_HOST_MEMORY): return "ERROR_OUT_OF_HOST_MEMORY";
        case (VK_ERROR_OUT_OF_DEVICE_MEMORY): return "ERROR_OUT_OF_DEVICE_MEMORY";
        case (VK_ERROR_INITIALIZATION_FAILED): return "ERROR_INITIALIZATION_FAILED";
        case (VK_ERROR_DEVICE_LOST): return "ERROR_DEVICE_LOST";
        case (VK_ERROR_MEMORY_MAP_FAILED): return "ERROR_MEMORY_MAP_FAILED";
        case (VK_ERROR_LAYER_NOT_PRESENT): return "ERROR_LAYER_NOT_PRESENT";
        case (VK_ERROR_EXTENSION_NOT_PRESENT): return "ERROR_EXTENSION_NOT_PRESENT";
        case (VK_ERROR_FEATURE_NOT_PRESENT): return "ERROR_FEATURE_NOT_PRESENT";
        case (VK_ERROR_INCOMPATIBLE_DRIVER): return "ERROR_INCOMPATIBLE_DRIVER";
        case (VK_ERROR_TOO_MANY_OBJECTS): return "ERROR_TOO_MANY_OBJECTS";
        case (VK_ERROR_FORMAT_NOT_SUPPORTED): return "ERROR_FORMAT_NOT_SUPPORTED";
        case (VK_ERROR_FRAGMENTED_POOL): return "ERROR_FRAGMENTED_POOL";
        case (VK_ERROR_UNKNOWN): return "ERROR_UNKNOWN";
        case (VK_ERROR_OUT_OF_POOL_MEMORY): return "ERROR_OUT_OF_POOL_MEMORY";
        case (VK_ERROR_INVALID_EXTERNAL_HANDLE): return "ERROR_INVALID_EXTERNAL_HANDLE";
        case (VK_ERROR_FRAGMENTATION): return "ERROR_FRAGMENTATION";
        case (VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS): return "ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case (VK_PIPELINE_COMPILE_REQUIRED): return "PIPELINE_COMPILE_REQUIRED";
        case (VK_ERROR_NOT_PERMITTED): return "ERROR_NOT_PERMITTED";
        case (VK_ERROR_SURFACE_LOST_KHR): return "ERROR_SURFACE_LOST_KHR";
        case (VK_ERROR_NATIVE_WINDOW_IN_USE_KHR): return "ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case (VK_SUBOPTIMAL_KHR): return "SUBOPTIMAL_KHR";
        case (VK_ERROR_OUT_OF_DATE_KHR): return "ERROR_OUT_OF_DATE_KHR";
        case (VK_ERROR_INCOMPATIBLE_DISPLAY_KHR): return "ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case (VK_ERROR_VALIDATION_FAILED_EXT): return "ERROR_VALIDATION_FAILED_EXT";
        case (VK_ERROR_INVALID_SHADER_NV): return "ERROR_INVALID_SHADER_NV";
        case (VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR): return "ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR): return "ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR): return "ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR): return "ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR): return "ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR): return "ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
        case (VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT): return "ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case (VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT): return "ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case (VK_THREAD_IDLE_KHR): return "THREAD_IDLE_KHR";
        case (VK_THREAD_DONE_KHR): return "THREAD_DONE_KHR";
        case (VK_OPERATION_DEFERRED_KHR): return "OPERATION_DEFERRED_KHR";
        case (VK_OPERATION_NOT_DEFERRED_KHR): return "OPERATION_NOT_DEFERRED_KHR";
        case (VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR): return "ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
        case (VK_ERROR_COMPRESSION_EXHAUSTED_EXT): return "ERROR_COMPRESSION_EXHAUSTED_EXT";
        case (VK_INCOMPATIBLE_SHADER_BINARY_EXT): return "INCOMPATIBLE_SHADER_BINARY_EXT";
        case (VK_PIPELINE_BINARY_MISSING_KHR): return "PIPELINE_BINARY_MISSING_KHR";
        case (VK_ERROR_NOT_ENOUGH_SPACE_KHR): return "ERROR_NOT_ENOUGH_SPACE_KHR";
        default: return std::string("UNKNOWN_VkResult_value") + std::to_string(value);
    }
}
void DumpVkResult(Printer &p, std::string name, VkResult value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkResultString(value));
    else
        p.PrintKeyString(name, VkResultString(value));
}
std::string VkShaderFloatControlsIndependenceString(VkShaderFloatControlsIndependence value) {
    switch (value) {
        case (VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY): return "SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY";
        case (VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL): return "SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL";
        case (VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE): return "SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE";
        default: return std::string("UNKNOWN_VkShaderFloatControlsIndependence_value") + std::to_string(value);
    }
}
void DumpVkShaderFloatControlsIndependence(Printer &p, std::string name, VkShaderFloatControlsIndependence value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkShaderFloatControlsIndependenceString(value));
    else
        p.PrintKeyString(name, VkShaderFloatControlsIndependenceString(value));
}
std::string VkVideoEncodeTuningModeKHRString(VkVideoEncodeTuningModeKHR value) {
    switch (value) {
        case (VK_VIDEO_ENCODE_TUNING_MODE_DEFAULT_KHR): return "VIDEO_ENCODE_TUNING_MODE_DEFAULT_KHR";
        case (VK_VIDEO_ENCODE_TUNING_MODE_HIGH_QUALITY_KHR): return "VIDEO_ENCODE_TUNING_MODE_HIGH_QUALITY_KHR";
        case (VK_VIDEO_ENCODE_TUNING_MODE_LOW_LATENCY_KHR): return "VIDEO_ENCODE_TUNING_MODE_LOW_LATENCY_KHR";
        case (VK_VIDEO_ENCODE_TUNING_MODE_ULTRA_LOW_LATENCY_KHR): return "VIDEO_ENCODE_TUNING_MODE_ULTRA_LOW_LATENCY_KHR";
        case (VK_VIDEO_ENCODE_TUNING_MODE_LOSSLESS_KHR): return "VIDEO_ENCODE_TUNING_MODE_LOSSLESS_KHR";
        default: return std::string("UNKNOWN_VkVideoEncodeTuningModeKHR_value") + std::to_string(value);
    }
}
void DumpVkVideoEncodeTuningModeKHR(Printer &p, std::string name, VkVideoEncodeTuningModeKHR value) {
    if (p.Type() == OutputType::json)
        p.PrintKeyString(name, std::string("VK_") + VkVideoEncodeTuningModeKHRString(value));
    else
        p.PrintKeyString(name, VkVideoEncodeTuningModeKHRString(value));
}
std::vector<const char *> VkCompositeAlphaFlagBitsKHRGetStrings(VkCompositeAlphaFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR & value) strings.push_back("COMPOSITE_ALPHA_OPAQUE_BIT_KHR");
    if (VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR & value) strings.push_back("COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR");
    if (VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR & value) strings.push_back("COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR");
    if (VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR & value) strings.push_back("COMPOSITE_ALPHA_INHERIT_BIT_KHR");
    return strings;
}
void DumpVkCompositeAlphaFlagsKHR(Printer &p, std::string name, VkCompositeAlphaFlagsKHR value) {
    if (static_cast<VkCompositeAlphaFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkCompositeAlphaFlagBitsKHRGetStrings(static_cast<VkCompositeAlphaFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkCompositeAlphaFlagBitsKHR(Printer &p, std::string name, VkCompositeAlphaFlagBitsKHR value) {
    auto strings = VkCompositeAlphaFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkDeviceGroupPresentModeFlagBitsKHRGetStrings(VkDeviceGroupPresentModeFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR & value) strings.push_back("DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR");
    if (VK_DEVICE_GROUP_PRESENT_MODE_REMOTE_BIT_KHR & value) strings.push_back("DEVICE_GROUP_PRESENT_MODE_REMOTE_BIT_KHR");
    if (VK_DEVICE_GROUP_PRESENT_MODE_SUM_BIT_KHR & value) strings.push_back("DEVICE_GROUP_PRESENT_MODE_SUM_BIT_KHR");
    if (VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT_KHR & value) strings.push_back("DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT_KHR");
    return strings;
}
void DumpVkDeviceGroupPresentModeFlagsKHR(Printer &p, std::string name, VkDeviceGroupPresentModeFlagsKHR value) {
    if (static_cast<VkDeviceGroupPresentModeFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkDeviceGroupPresentModeFlagBitsKHRGetStrings(static_cast<VkDeviceGroupPresentModeFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkDeviceGroupPresentModeFlagBitsKHR(Printer &p, std::string name, VkDeviceGroupPresentModeFlagBitsKHR value) {
    auto strings = VkDeviceGroupPresentModeFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkFormatFeatureFlagBitsGetStrings(VkFormatFeatureFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_BIT");
    if (VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & value) strings.push_back("FORMAT_FEATURE_STORAGE_IMAGE_BIT");
    if (VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT & value) strings.push_back("FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT");
    if (VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT & value) strings.push_back("FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT");
    if (VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_VERTEX_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT & value) strings.push_back("FORMAT_FEATURE_COLOR_ATTACHMENT_BIT");
    if (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT & value) strings.push_back("FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT");
    if (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT & value) strings.push_back("FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT");
    if (VK_FORMAT_FEATURE_BLIT_SRC_BIT & value) strings.push_back("FORMAT_FEATURE_BLIT_SRC_BIT");
    if (VK_FORMAT_FEATURE_BLIT_DST_BIT & value) strings.push_back("FORMAT_FEATURE_BLIT_DST_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT");
    if (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & value) strings.push_back("FORMAT_FEATURE_TRANSFER_SRC_BIT");
    if (VK_FORMAT_FEATURE_TRANSFER_DST_BIT & value) strings.push_back("FORMAT_FEATURE_TRANSFER_DST_BIT");
    if (VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT & value) strings.push_back("FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT");
    if (VK_FORMAT_FEATURE_DISJOINT_BIT & value) strings.push_back("FORMAT_FEATURE_DISJOINT_BIT");
    if (VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT & value) strings.push_back("FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT");
    if (VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR");
    if (VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR");
    if (VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT");
    if (VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT & value) strings.push_back("FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT");
    if (VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR");
    if (VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR");
    if (VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR");
    return strings;
}
void DumpVkFormatFeatureFlags(Printer &p, std::string name, VkFormatFeatureFlags value) {
    if (static_cast<VkFormatFeatureFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkFormatFeatureFlagBitsGetStrings(static_cast<VkFormatFeatureFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkFormatFeatureFlagBits(Printer &p, std::string name, VkFormatFeatureFlagBits value) {
    auto strings = VkFormatFeatureFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkFormatFeatureFlagBits2GetStrings(VkFormatFeatureFlagBits2 value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT");
    if (VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT & value) strings.push_back("FORMAT_FEATURE_2_STORAGE_IMAGE_BIT");
    if (VK_FORMAT_FEATURE_2_STORAGE_IMAGE_ATOMIC_BIT & value) strings.push_back("FORMAT_FEATURE_2_STORAGE_IMAGE_ATOMIC_BIT");
    if (VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT & value) strings.push_back("FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT");
    if (VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_2_VERTEX_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT & value) strings.push_back("FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT");
    if (VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT & value) strings.push_back("FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT");
    if (VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT & value) strings.push_back("FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT");
    if (VK_FORMAT_FEATURE_2_BLIT_SRC_BIT & value) strings.push_back("FORMAT_FEATURE_2_BLIT_SRC_BIT");
    if (VK_FORMAT_FEATURE_2_BLIT_DST_BIT & value) strings.push_back("FORMAT_FEATURE_2_BLIT_DST_BIT");
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT");
    if (VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT & value) strings.push_back("FORMAT_FEATURE_2_TRANSFER_SRC_BIT");
    if (VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT & value) strings.push_back("FORMAT_FEATURE_2_TRANSFER_DST_BIT");
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_MINMAX_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_MINMAX_BIT");
    if (VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT & value) strings.push_back("FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT");
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT");
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT");
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT");
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT");
    if (VK_FORMAT_FEATURE_2_DISJOINT_BIT & value) strings.push_back("FORMAT_FEATURE_2_DISJOINT_BIT");
    if (VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT & value) strings.push_back("FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT");
    if (VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT & value) strings.push_back("FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT");
    if (VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT & value) strings.push_back("FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT");
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT");
    if (VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_CUBIC_BIT & value) strings.push_back("FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_CUBIC_BIT");
    if (VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT & value) strings.push_back("FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT");
    if (VK_FORMAT_FEATURE_2_VIDEO_DECODE_OUTPUT_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_2_VIDEO_DECODE_OUTPUT_BIT_KHR");
    if (VK_FORMAT_FEATURE_2_VIDEO_DECODE_DPB_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_2_VIDEO_DECODE_DPB_BIT_KHR");
    if (VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR");
    if (VK_FORMAT_FEATURE_2_FRAGMENT_DENSITY_MAP_BIT_EXT & value) strings.push_back("FORMAT_FEATURE_2_FRAGMENT_DENSITY_MAP_BIT_EXT");
    if (VK_FORMAT_FEATURE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR");
    if (VK_FORMAT_FEATURE_2_VIDEO_ENCODE_INPUT_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_2_VIDEO_ENCODE_INPUT_BIT_KHR");
    if (VK_FORMAT_FEATURE_2_VIDEO_ENCODE_DPB_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_2_VIDEO_ENCODE_DPB_BIT_KHR");
    if (VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV & value) strings.push_back("FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV");
    if (VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV & value) strings.push_back("FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV");
    if (VK_FORMAT_FEATURE_2_WEIGHT_IMAGE_BIT_QCOM & value) strings.push_back("FORMAT_FEATURE_2_WEIGHT_IMAGE_BIT_QCOM");
    if (VK_FORMAT_FEATURE_2_WEIGHT_SAMPLED_IMAGE_BIT_QCOM & value) strings.push_back("FORMAT_FEATURE_2_WEIGHT_SAMPLED_IMAGE_BIT_QCOM");
    if (VK_FORMAT_FEATURE_2_BLOCK_MATCHING_BIT_QCOM & value) strings.push_back("FORMAT_FEATURE_2_BLOCK_MATCHING_BIT_QCOM");
    if (VK_FORMAT_FEATURE_2_BOX_FILTER_SAMPLED_BIT_QCOM & value) strings.push_back("FORMAT_FEATURE_2_BOX_FILTER_SAMPLED_BIT_QCOM");
    if (VK_FORMAT_FEATURE_2_OPTICAL_FLOW_IMAGE_BIT_NV & value) strings.push_back("FORMAT_FEATURE_2_OPTICAL_FLOW_IMAGE_BIT_NV");
    if (VK_FORMAT_FEATURE_2_OPTICAL_FLOW_VECTOR_BIT_NV & value) strings.push_back("FORMAT_FEATURE_2_OPTICAL_FLOW_VECTOR_BIT_NV");
    if (VK_FORMAT_FEATURE_2_OPTICAL_FLOW_COST_BIT_NV & value) strings.push_back("FORMAT_FEATURE_2_OPTICAL_FLOW_COST_BIT_NV");
    if (VK_FORMAT_FEATURE_2_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_2_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR");
    if (VK_FORMAT_FEATURE_2_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_2_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR");
    return strings;
}
void DumpVkFormatFeatureFlags2(Printer &p, std::string name, VkFormatFeatureFlags2 value) {
    if (static_cast<VkFormatFeatureFlagBits2>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkFormatFeatureFlagBits2GetStrings(static_cast<VkFormatFeatureFlagBits2>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkFormatFeatureFlagBits2(Printer &p, std::string name, VkFormatFeatureFlagBits2 value) {
    auto strings = VkFormatFeatureFlagBits2GetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkImageCreateFlagBitsGetStrings(VkImageCreateFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_IMAGE_CREATE_SPARSE_BINDING_BIT & value) strings.push_back("IMAGE_CREATE_SPARSE_BINDING_BIT");
    if (VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT & value) strings.push_back("IMAGE_CREATE_SPARSE_RESIDENCY_BIT");
    if (VK_IMAGE_CREATE_SPARSE_ALIASED_BIT & value) strings.push_back("IMAGE_CREATE_SPARSE_ALIASED_BIT");
    if (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT & value) strings.push_back("IMAGE_CREATE_MUTABLE_FORMAT_BIT");
    if (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT & value) strings.push_back("IMAGE_CREATE_CUBE_COMPATIBLE_BIT");
    if (VK_IMAGE_CREATE_ALIAS_BIT & value) strings.push_back("IMAGE_CREATE_ALIAS_BIT");
    if (VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT & value) strings.push_back("IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT");
    if (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT & value) strings.push_back("IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT");
    if (VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT & value) strings.push_back("IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT");
    if (VK_IMAGE_CREATE_EXTENDED_USAGE_BIT & value) strings.push_back("IMAGE_CREATE_EXTENDED_USAGE_BIT");
    if (VK_IMAGE_CREATE_PROTECTED_BIT & value) strings.push_back("IMAGE_CREATE_PROTECTED_BIT");
    if (VK_IMAGE_CREATE_DISJOINT_BIT & value) strings.push_back("IMAGE_CREATE_DISJOINT_BIT");
    if (VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV & value) strings.push_back("IMAGE_CREATE_CORNER_SAMPLED_BIT_NV");
    if (VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT & value) strings.push_back("IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT");
    if (VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT & value) strings.push_back("IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
    if (VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT & value) strings.push_back("IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT");
    if (VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT & value) strings.push_back("IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT");
    if (VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT & value) strings.push_back("IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT");
    if (VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM & value) strings.push_back("IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM");
    if (VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR & value) strings.push_back("IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR");
    return strings;
}
void DumpVkImageCreateFlags(Printer &p, std::string name, VkImageCreateFlags value) {
    if (static_cast<VkImageCreateFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkImageCreateFlagBitsGetStrings(static_cast<VkImageCreateFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkImageCreateFlagBits(Printer &p, std::string name, VkImageCreateFlagBits value) {
    auto strings = VkImageCreateFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkImageUsageFlagBitsGetStrings(VkImageUsageFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_IMAGE_USAGE_TRANSFER_SRC_BIT & value) strings.push_back("IMAGE_USAGE_TRANSFER_SRC_BIT");
    if (VK_IMAGE_USAGE_TRANSFER_DST_BIT & value) strings.push_back("IMAGE_USAGE_TRANSFER_DST_BIT");
    if (VK_IMAGE_USAGE_SAMPLED_BIT & value) strings.push_back("IMAGE_USAGE_SAMPLED_BIT");
    if (VK_IMAGE_USAGE_STORAGE_BIT & value) strings.push_back("IMAGE_USAGE_STORAGE_BIT");
    if (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT & value) strings.push_back("IMAGE_USAGE_COLOR_ATTACHMENT_BIT");
    if (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT & value) strings.push_back("IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT");
    if (VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT & value) strings.push_back("IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT");
    if (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT & value) strings.push_back("IMAGE_USAGE_INPUT_ATTACHMENT_BIT");
    if (VK_IMAGE_USAGE_HOST_TRANSFER_BIT & value) strings.push_back("IMAGE_USAGE_HOST_TRANSFER_BIT");
    if (VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR & value) strings.push_back("IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR");
    if (VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR & value) strings.push_back("IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR");
    if (VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR & value) strings.push_back("IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR");
    if (VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT & value) strings.push_back("IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT");
    if (VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR & value) strings.push_back("IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR");
    if (VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR & value) strings.push_back("IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR");
    if (VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR & value) strings.push_back("IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR");
    if (VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR & value) strings.push_back("IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR");
    if (VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT & value) strings.push_back("IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT");
    if (VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI & value) strings.push_back("IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI");
    if (VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM & value) strings.push_back("IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM");
    if (VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM & value) strings.push_back("IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM");
    if (VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR & value) strings.push_back("IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR");
    if (VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR & value) strings.push_back("IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR");
    return strings;
}
void DumpVkImageUsageFlags(Printer &p, std::string name, VkImageUsageFlags value) {
    if (static_cast<VkImageUsageFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkImageUsageFlagBitsGetStrings(static_cast<VkImageUsageFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkImageUsageFlagBits(Printer &p, std::string name, VkImageUsageFlagBits value) {
    auto strings = VkImageUsageFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkIndirectCommandsInputModeFlagBitsEXTGetStrings(VkIndirectCommandsInputModeFlagBitsEXT value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_INDIRECT_COMMANDS_INPUT_MODE_VULKAN_INDEX_BUFFER_EXT & value) strings.push_back("INDIRECT_COMMANDS_INPUT_MODE_VULKAN_INDEX_BUFFER_EXT");
    if (VK_INDIRECT_COMMANDS_INPUT_MODE_DXGI_INDEX_BUFFER_EXT & value) strings.push_back("INDIRECT_COMMANDS_INPUT_MODE_DXGI_INDEX_BUFFER_EXT");
    return strings;
}
void DumpVkIndirectCommandsInputModeFlagsEXT(Printer &p, std::string name, VkIndirectCommandsInputModeFlagsEXT value) {
    if (static_cast<VkIndirectCommandsInputModeFlagBitsEXT>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkIndirectCommandsInputModeFlagBitsEXTGetStrings(static_cast<VkIndirectCommandsInputModeFlagBitsEXT>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkIndirectCommandsInputModeFlagBitsEXT(Printer &p, std::string name, VkIndirectCommandsInputModeFlagBitsEXT value) {
    auto strings = VkIndirectCommandsInputModeFlagBitsEXTGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkMemoryHeapFlagBitsGetStrings(VkMemoryHeapFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & value) strings.push_back("MEMORY_HEAP_DEVICE_LOCAL_BIT");
    if (VK_MEMORY_HEAP_MULTI_INSTANCE_BIT & value) strings.push_back("MEMORY_HEAP_MULTI_INSTANCE_BIT");
    return strings;
}
void DumpVkMemoryHeapFlags(Printer &p, std::string name, VkMemoryHeapFlags value) {
    if (static_cast<VkMemoryHeapFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkMemoryHeapFlagBitsGetStrings(static_cast<VkMemoryHeapFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkMemoryHeapFlagBits(Printer &p, std::string name, VkMemoryHeapFlagBits value) {
    auto strings = VkMemoryHeapFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkMemoryPropertyFlagBitsGetStrings(VkMemoryPropertyFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & value) strings.push_back("MEMORY_PROPERTY_DEVICE_LOCAL_BIT");
    if (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & value) strings.push_back("MEMORY_PROPERTY_HOST_VISIBLE_BIT");
    if (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & value) strings.push_back("MEMORY_PROPERTY_HOST_COHERENT_BIT");
    if (VK_MEMORY_PROPERTY_HOST_CACHED_BIT & value) strings.push_back("MEMORY_PROPERTY_HOST_CACHED_BIT");
    if (VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT & value) strings.push_back("MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT");
    if (VK_MEMORY_PROPERTY_PROTECTED_BIT & value) strings.push_back("MEMORY_PROPERTY_PROTECTED_BIT");
    if (VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD & value) strings.push_back("MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD");
    if (VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD & value) strings.push_back("MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD");
    if (VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV & value) strings.push_back("MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV");
    return strings;
}
void DumpVkMemoryPropertyFlags(Printer &p, std::string name, VkMemoryPropertyFlags value) {
    if (static_cast<VkMemoryPropertyFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkMemoryPropertyFlagBitsGetStrings(static_cast<VkMemoryPropertyFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkMemoryPropertyFlagBits(Printer &p, std::string name, VkMemoryPropertyFlagBits value) {
    auto strings = VkMemoryPropertyFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkPresentGravityFlagBitsEXTGetStrings(VkPresentGravityFlagBitsEXT value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_PRESENT_GRAVITY_MIN_BIT_EXT & value) strings.push_back("PRESENT_GRAVITY_MIN_BIT_EXT");
    if (VK_PRESENT_GRAVITY_MAX_BIT_EXT & value) strings.push_back("PRESENT_GRAVITY_MAX_BIT_EXT");
    if (VK_PRESENT_GRAVITY_CENTERED_BIT_EXT & value) strings.push_back("PRESENT_GRAVITY_CENTERED_BIT_EXT");
    return strings;
}
void DumpVkPresentGravityFlagsEXT(Printer &p, std::string name, VkPresentGravityFlagsEXT value) {
    if (static_cast<VkPresentGravityFlagBitsEXT>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkPresentGravityFlagBitsEXTGetStrings(static_cast<VkPresentGravityFlagBitsEXT>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkPresentGravityFlagBitsEXT(Printer &p, std::string name, VkPresentGravityFlagBitsEXT value) {
    auto strings = VkPresentGravityFlagBitsEXTGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkPresentScalingFlagBitsEXTGetStrings(VkPresentScalingFlagBitsEXT value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT & value) strings.push_back("PRESENT_SCALING_ONE_TO_ONE_BIT_EXT");
    if (VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT & value) strings.push_back("PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT");
    if (VK_PRESENT_SCALING_STRETCH_BIT_EXT & value) strings.push_back("PRESENT_SCALING_STRETCH_BIT_EXT");
    return strings;
}
void DumpVkPresentScalingFlagsEXT(Printer &p, std::string name, VkPresentScalingFlagsEXT value) {
    if (static_cast<VkPresentScalingFlagBitsEXT>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkPresentScalingFlagBitsEXTGetStrings(static_cast<VkPresentScalingFlagBitsEXT>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkPresentScalingFlagBitsEXT(Printer &p, std::string name, VkPresentScalingFlagBitsEXT value) {
    auto strings = VkPresentScalingFlagBitsEXTGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkQueueFlagBitsGetStrings(VkQueueFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_QUEUE_GRAPHICS_BIT & value) strings.push_back("QUEUE_GRAPHICS_BIT");
    if (VK_QUEUE_COMPUTE_BIT & value) strings.push_back("QUEUE_COMPUTE_BIT");
    if (VK_QUEUE_TRANSFER_BIT & value) strings.push_back("QUEUE_TRANSFER_BIT");
    if (VK_QUEUE_SPARSE_BINDING_BIT & value) strings.push_back("QUEUE_SPARSE_BINDING_BIT");
    if (VK_QUEUE_PROTECTED_BIT & value) strings.push_back("QUEUE_PROTECTED_BIT");
    if (VK_QUEUE_VIDEO_DECODE_BIT_KHR & value) strings.push_back("QUEUE_VIDEO_DECODE_BIT_KHR");
    if (VK_QUEUE_VIDEO_ENCODE_BIT_KHR & value) strings.push_back("QUEUE_VIDEO_ENCODE_BIT_KHR");
    if (VK_QUEUE_OPTICAL_FLOW_BIT_NV & value) strings.push_back("QUEUE_OPTICAL_FLOW_BIT_NV");
    return strings;
}
void DumpVkQueueFlags(Printer &p, std::string name, VkQueueFlags value) {
    if (static_cast<VkQueueFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkQueueFlagBitsGetStrings(static_cast<VkQueueFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkQueueFlagBits(Printer &p, std::string name, VkQueueFlagBits value) {
    auto strings = VkQueueFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::string VkQueueFlagsString(VkQueueFlags value) {
    std::string out;
    bool is_first = true;
    if (VK_QUEUE_GRAPHICS_BIT & value) {
        if (is_first) { is_first = false; } else { out += " | "; }
        out += "QUEUE_GRAPHICS_BIT";
    }
    if (VK_QUEUE_COMPUTE_BIT & value) {
        if (is_first) { is_first = false; } else { out += " | "; }
        out += "QUEUE_COMPUTE_BIT";
    }
    if (VK_QUEUE_TRANSFER_BIT & value) {
        if (is_first) { is_first = false; } else { out += " | "; }
        out += "QUEUE_TRANSFER_BIT";
    }
    if (VK_QUEUE_SPARSE_BINDING_BIT & value) {
        if (is_first) { is_first = false; } else { out += " | "; }
        out += "QUEUE_SPARSE_BINDING_BIT";
    }
    if (VK_QUEUE_PROTECTED_BIT & value) {
        if (is_first) { is_first = false; } else { out += " | "; }
        out += "QUEUE_PROTECTED_BIT";
    }
    if (VK_QUEUE_VIDEO_DECODE_BIT_KHR & value) {
        if (is_first) { is_first = false; } else { out += " | "; }
        out += "QUEUE_VIDEO_DECODE_BIT_KHR";
    }
    if (VK_QUEUE_VIDEO_ENCODE_BIT_KHR & value) {
        if (is_first) { is_first = false; } else { out += " | "; }
        out += "QUEUE_VIDEO_ENCODE_BIT_KHR";
    }
    if (VK_QUEUE_OPTICAL_FLOW_BIT_NV & value) {
        if (is_first) { is_first = false; } else { out += " | "; }
        out += "QUEUE_OPTICAL_FLOW_BIT_NV";
    }
    return out;
}
std::vector<const char *> VkResolveModeFlagBitsGetStrings(VkResolveModeFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("RESOLVE_MODE_NONE"); return strings; }
    if (VK_RESOLVE_MODE_SAMPLE_ZERO_BIT & value) strings.push_back("RESOLVE_MODE_SAMPLE_ZERO_BIT");
    if (VK_RESOLVE_MODE_AVERAGE_BIT & value) strings.push_back("RESOLVE_MODE_AVERAGE_BIT");
    if (VK_RESOLVE_MODE_MIN_BIT & value) strings.push_back("RESOLVE_MODE_MIN_BIT");
    if (VK_RESOLVE_MODE_MAX_BIT & value) strings.push_back("RESOLVE_MODE_MAX_BIT");
    if (VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID & value) strings.push_back("RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID");
    return strings;
}
void DumpVkResolveModeFlags(Printer &p, std::string name, VkResolveModeFlags value) {
    if (static_cast<VkResolveModeFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkResolveModeFlagBitsGetStrings(static_cast<VkResolveModeFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkResolveModeFlagBits(Printer &p, std::string name, VkResolveModeFlagBits value) {
    auto strings = VkResolveModeFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkSampleCountFlagBitsGetStrings(VkSampleCountFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_SAMPLE_COUNT_1_BIT & value) strings.push_back("SAMPLE_COUNT_1_BIT");
    if (VK_SAMPLE_COUNT_2_BIT & value) strings.push_back("SAMPLE_COUNT_2_BIT");
    if (VK_SAMPLE_COUNT_4_BIT & value) strings.push_back("SAMPLE_COUNT_4_BIT");
    if (VK_SAMPLE_COUNT_8_BIT & value) strings.push_back("SAMPLE_COUNT_8_BIT");
    if (VK_SAMPLE_COUNT_16_BIT & value) strings.push_back("SAMPLE_COUNT_16_BIT");
    if (VK_SAMPLE_COUNT_32_BIT & value) strings.push_back("SAMPLE_COUNT_32_BIT");
    if (VK_SAMPLE_COUNT_64_BIT & value) strings.push_back("SAMPLE_COUNT_64_BIT");
    return strings;
}
void DumpVkSampleCountFlags(Printer &p, std::string name, VkSampleCountFlags value) {
    if (static_cast<VkSampleCountFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkSampleCountFlagBitsGetStrings(static_cast<VkSampleCountFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkSampleCountFlagBits(Printer &p, std::string name, VkSampleCountFlagBits value) {
    auto strings = VkSampleCountFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkShaderStageFlagBitsGetStrings(VkShaderStageFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_SHADER_STAGE_VERTEX_BIT & value) strings.push_back("SHADER_STAGE_VERTEX_BIT");
    if (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT & value) strings.push_back("SHADER_STAGE_TESSELLATION_CONTROL_BIT");
    if (VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT & value) strings.push_back("SHADER_STAGE_TESSELLATION_EVALUATION_BIT");
    if (VK_SHADER_STAGE_GEOMETRY_BIT & value) strings.push_back("SHADER_STAGE_GEOMETRY_BIT");
    if (VK_SHADER_STAGE_FRAGMENT_BIT & value) strings.push_back("SHADER_STAGE_FRAGMENT_BIT");
    if (VK_SHADER_STAGE_COMPUTE_BIT & value) strings.push_back("SHADER_STAGE_COMPUTE_BIT");
    if (VK_SHADER_STAGE_RAYGEN_BIT_KHR & value) strings.push_back("SHADER_STAGE_RAYGEN_BIT_KHR");
    if (VK_SHADER_STAGE_ANY_HIT_BIT_KHR & value) strings.push_back("SHADER_STAGE_ANY_HIT_BIT_KHR");
    if (VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR & value) strings.push_back("SHADER_STAGE_CLOSEST_HIT_BIT_KHR");
    if (VK_SHADER_STAGE_MISS_BIT_KHR & value) strings.push_back("SHADER_STAGE_MISS_BIT_KHR");
    if (VK_SHADER_STAGE_INTERSECTION_BIT_KHR & value) strings.push_back("SHADER_STAGE_INTERSECTION_BIT_KHR");
    if (VK_SHADER_STAGE_CALLABLE_BIT_KHR & value) strings.push_back("SHADER_STAGE_CALLABLE_BIT_KHR");
    if (VK_SHADER_STAGE_TASK_BIT_EXT & value) strings.push_back("SHADER_STAGE_TASK_BIT_EXT");
    if (VK_SHADER_STAGE_MESH_BIT_EXT & value) strings.push_back("SHADER_STAGE_MESH_BIT_EXT");
    if (VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI & value) strings.push_back("SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI");
    if (VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI & value) strings.push_back("SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI");
    return strings;
}
void DumpVkShaderStageFlags(Printer &p, std::string name, VkShaderStageFlags value) {
    if (static_cast<VkShaderStageFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkShaderStageFlagBitsGetStrings(static_cast<VkShaderStageFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkShaderStageFlagBits(Printer &p, std::string name, VkShaderStageFlagBits value) {
    auto strings = VkShaderStageFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkSubgroupFeatureFlagBitsGetStrings(VkSubgroupFeatureFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_SUBGROUP_FEATURE_BASIC_BIT & value) strings.push_back("SUBGROUP_FEATURE_BASIC_BIT");
    if (VK_SUBGROUP_FEATURE_VOTE_BIT & value) strings.push_back("SUBGROUP_FEATURE_VOTE_BIT");
    if (VK_SUBGROUP_FEATURE_ARITHMETIC_BIT & value) strings.push_back("SUBGROUP_FEATURE_ARITHMETIC_BIT");
    if (VK_SUBGROUP_FEATURE_BALLOT_BIT & value) strings.push_back("SUBGROUP_FEATURE_BALLOT_BIT");
    if (VK_SUBGROUP_FEATURE_SHUFFLE_BIT & value) strings.push_back("SUBGROUP_FEATURE_SHUFFLE_BIT");
    if (VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT & value) strings.push_back("SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT");
    if (VK_SUBGROUP_FEATURE_CLUSTERED_BIT & value) strings.push_back("SUBGROUP_FEATURE_CLUSTERED_BIT");
    if (VK_SUBGROUP_FEATURE_QUAD_BIT & value) strings.push_back("SUBGROUP_FEATURE_QUAD_BIT");
    if (VK_SUBGROUP_FEATURE_ROTATE_BIT & value) strings.push_back("SUBGROUP_FEATURE_ROTATE_BIT");
    if (VK_SUBGROUP_FEATURE_ROTATE_CLUSTERED_BIT & value) strings.push_back("SUBGROUP_FEATURE_ROTATE_CLUSTERED_BIT");
    if (VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV & value) strings.push_back("SUBGROUP_FEATURE_PARTITIONED_BIT_NV");
    return strings;
}
void DumpVkSubgroupFeatureFlags(Printer &p, std::string name, VkSubgroupFeatureFlags value) {
    if (static_cast<VkSubgroupFeatureFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkSubgroupFeatureFlagBitsGetStrings(static_cast<VkSubgroupFeatureFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkSubgroupFeatureFlagBits(Printer &p, std::string name, VkSubgroupFeatureFlagBits value) {
    auto strings = VkSubgroupFeatureFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkSurfaceCounterFlagBitsEXTGetStrings(VkSurfaceCounterFlagBitsEXT value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_SURFACE_COUNTER_VBLANK_BIT_EXT & value) strings.push_back("SURFACE_COUNTER_VBLANK_BIT_EXT");
    return strings;
}
void DumpVkSurfaceCounterFlagsEXT(Printer &p, std::string name, VkSurfaceCounterFlagsEXT value) {
    if (static_cast<VkSurfaceCounterFlagBitsEXT>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkSurfaceCounterFlagBitsEXTGetStrings(static_cast<VkSurfaceCounterFlagBitsEXT>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkSurfaceCounterFlagBitsEXT(Printer &p, std::string name, VkSurfaceCounterFlagBitsEXT value) {
    auto strings = VkSurfaceCounterFlagBitsEXTGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkSurfaceTransformFlagBitsKHRGetStrings(VkSurfaceTransformFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_IDENTITY_BIT_KHR");
    if (VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_ROTATE_90_BIT_KHR");
    if (VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_ROTATE_180_BIT_KHR");
    if (VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_ROTATE_270_BIT_KHR");
    if (VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR");
    if (VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR");
    if (VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR");
    if (VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR");
    if (VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR & value) strings.push_back("SURFACE_TRANSFORM_INHERIT_BIT_KHR");
    return strings;
}
void DumpVkSurfaceTransformFlagsKHR(Printer &p, std::string name, VkSurfaceTransformFlagsKHR value) {
    if (static_cast<VkSurfaceTransformFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkSurfaceTransformFlagBitsKHRGetStrings(static_cast<VkSurfaceTransformFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkSurfaceTransformFlagBitsKHR(Printer &p, std::string name, VkSurfaceTransformFlagBitsKHR value) {
    auto strings = VkSurfaceTransformFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkToolPurposeFlagBitsGetStrings(VkToolPurposeFlagBits value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_TOOL_PURPOSE_VALIDATION_BIT & value) strings.push_back("TOOL_PURPOSE_VALIDATION_BIT");
    if (VK_TOOL_PURPOSE_PROFILING_BIT & value) strings.push_back("TOOL_PURPOSE_PROFILING_BIT");
    if (VK_TOOL_PURPOSE_TRACING_BIT & value) strings.push_back("TOOL_PURPOSE_TRACING_BIT");
    if (VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT & value) strings.push_back("TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT");
    if (VK_TOOL_PURPOSE_MODIFYING_FEATURES_BIT & value) strings.push_back("TOOL_PURPOSE_MODIFYING_FEATURES_BIT");
    if (VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT & value) strings.push_back("TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT");
    if (VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT & value) strings.push_back("TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT");
    return strings;
}
void DumpVkToolPurposeFlags(Printer &p, std::string name, VkToolPurposeFlags value) {
    if (static_cast<VkToolPurposeFlagBits>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkToolPurposeFlagBitsGetStrings(static_cast<VkToolPurposeFlagBits>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkToolPurposeFlagBits(Printer &p, std::string name, VkToolPurposeFlagBits value) {
    auto strings = VkToolPurposeFlagBitsGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoCapabilityFlagBitsKHRGetStrings(VkVideoCapabilityFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR & value) strings.push_back("VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR");
    if (VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR & value) strings.push_back("VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR");
    return strings;
}
void DumpVkVideoCapabilityFlagsKHR(Printer &p, std::string name, VkVideoCapabilityFlagsKHR value) {
    if (static_cast<VkVideoCapabilityFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoCapabilityFlagBitsKHRGetStrings(static_cast<VkVideoCapabilityFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoCapabilityFlagBitsKHR(Printer &p, std::string name, VkVideoCapabilityFlagBitsKHR value) {
    auto strings = VkVideoCapabilityFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoChromaSubsamplingFlagBitsKHRGetStrings(VkVideoChromaSubsamplingFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("VIDEO_CHROMA_SUBSAMPLING_INVALID_KHR"); return strings; }
    if (VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR & value) strings.push_back("VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR");
    if (VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR & value) strings.push_back("VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR");
    if (VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR & value) strings.push_back("VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR");
    if (VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR & value) strings.push_back("VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR");
    return strings;
}
void DumpVkVideoChromaSubsamplingFlagsKHR(Printer &p, std::string name, VkVideoChromaSubsamplingFlagsKHR value) {
    if (static_cast<VkVideoChromaSubsamplingFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoChromaSubsamplingFlagBitsKHRGetStrings(static_cast<VkVideoChromaSubsamplingFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoChromaSubsamplingFlagBitsKHR(Printer &p, std::string name, VkVideoChromaSubsamplingFlagBitsKHR value) {
    auto strings = VkVideoChromaSubsamplingFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoCodecOperationFlagBitsKHRGetStrings(VkVideoCodecOperationFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("VIDEO_CODEC_OPERATION_NONE_KHR"); return strings; }
    if (VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR & value) strings.push_back("VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR");
    if (VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR & value) strings.push_back("VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR");
    if (VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR & value) strings.push_back("VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR");
    if (VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR & value) strings.push_back("VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR");
    if (VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR & value) strings.push_back("VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR");
    if (VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR & value) strings.push_back("VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR");
    return strings;
}
void DumpVkVideoCodecOperationFlagsKHR(Printer &p, std::string name, VkVideoCodecOperationFlagsKHR value) {
    if (static_cast<VkVideoCodecOperationFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoCodecOperationFlagBitsKHRGetStrings(static_cast<VkVideoCodecOperationFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoCodecOperationFlagBitsKHR(Printer &p, std::string name, VkVideoCodecOperationFlagBitsKHR value) {
    auto strings = VkVideoCodecOperationFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoComponentBitDepthFlagBitsKHRGetStrings(VkVideoComponentBitDepthFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("VIDEO_COMPONENT_BIT_DEPTH_INVALID_KHR"); return strings; }
    if (VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR & value) strings.push_back("VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR");
    if (VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR & value) strings.push_back("VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR");
    if (VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR & value) strings.push_back("VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR");
    return strings;
}
void DumpVkVideoComponentBitDepthFlagsKHR(Printer &p, std::string name, VkVideoComponentBitDepthFlagsKHR value) {
    if (static_cast<VkVideoComponentBitDepthFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoComponentBitDepthFlagBitsKHRGetStrings(static_cast<VkVideoComponentBitDepthFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoComponentBitDepthFlagBitsKHR(Printer &p, std::string name, VkVideoComponentBitDepthFlagBitsKHR value) {
    auto strings = VkVideoComponentBitDepthFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoDecodeCapabilityFlagBitsKHRGetStrings(VkVideoDecodeCapabilityFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR & value) strings.push_back("VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR");
    if (VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR & value) strings.push_back("VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR");
    return strings;
}
void DumpVkVideoDecodeCapabilityFlagsKHR(Printer &p, std::string name, VkVideoDecodeCapabilityFlagsKHR value) {
    if (static_cast<VkVideoDecodeCapabilityFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoDecodeCapabilityFlagBitsKHRGetStrings(static_cast<VkVideoDecodeCapabilityFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoDecodeCapabilityFlagBitsKHR(Printer &p, std::string name, VkVideoDecodeCapabilityFlagBitsKHR value) {
    auto strings = VkVideoDecodeCapabilityFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoDecodeH264PictureLayoutFlagBitsKHRGetStrings(VkVideoDecodeH264PictureLayoutFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR"); return strings; }
    if (VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR & value) strings.push_back("VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR");
    if (VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR & value) strings.push_back("VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR");
    return strings;
}
void DumpVkVideoDecodeH264PictureLayoutFlagsKHR(Printer &p, std::string name, VkVideoDecodeH264PictureLayoutFlagsKHR value) {
    if (static_cast<VkVideoDecodeH264PictureLayoutFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoDecodeH264PictureLayoutFlagBitsKHRGetStrings(static_cast<VkVideoDecodeH264PictureLayoutFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoDecodeH264PictureLayoutFlagBitsKHR(Printer &p, std::string name, VkVideoDecodeH264PictureLayoutFlagBitsKHR value) {
    auto strings = VkVideoDecodeH264PictureLayoutFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoDecodeUsageFlagBitsKHRGetStrings(VkVideoDecodeUsageFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("VIDEO_DECODE_USAGE_DEFAULT_KHR"); return strings; }
    if (VK_VIDEO_DECODE_USAGE_TRANSCODING_BIT_KHR & value) strings.push_back("VIDEO_DECODE_USAGE_TRANSCODING_BIT_KHR");
    if (VK_VIDEO_DECODE_USAGE_OFFLINE_BIT_KHR & value) strings.push_back("VIDEO_DECODE_USAGE_OFFLINE_BIT_KHR");
    if (VK_VIDEO_DECODE_USAGE_STREAMING_BIT_KHR & value) strings.push_back("VIDEO_DECODE_USAGE_STREAMING_BIT_KHR");
    return strings;
}
void DumpVkVideoDecodeUsageFlagsKHR(Printer &p, std::string name, VkVideoDecodeUsageFlagsKHR value) {
    if (static_cast<VkVideoDecodeUsageFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoDecodeUsageFlagBitsKHRGetStrings(static_cast<VkVideoDecodeUsageFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoDecodeUsageFlagBitsKHR(Printer &p, std::string name, VkVideoDecodeUsageFlagBitsKHR value) {
    auto strings = VkVideoDecodeUsageFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeAV1CapabilityFlagBitsKHRGetStrings(VkVideoEncodeAV1CapabilityFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_AV1_CAPABILITY_PER_RATE_CONTROL_GROUP_MIN_MAX_Q_INDEX_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_CAPABILITY_PER_RATE_CONTROL_GROUP_MIN_MAX_Q_INDEX_BIT_KHR");
    if (VK_VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR");
    if (VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR");
    if (VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR");
    if (VK_VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeAV1CapabilityFlagsKHR(Printer &p, std::string name, VkVideoEncodeAV1CapabilityFlagsKHR value) {
    if (static_cast<VkVideoEncodeAV1CapabilityFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeAV1CapabilityFlagBitsKHRGetStrings(static_cast<VkVideoEncodeAV1CapabilityFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeAV1CapabilityFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeAV1CapabilityFlagBitsKHR value) {
    auto strings = VkVideoEncodeAV1CapabilityFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeAV1StdFlagBitsKHRGetStrings(VkVideoEncodeAV1StdFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_AV1_STD_UNIFORM_TILE_SPACING_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_STD_UNIFORM_TILE_SPACING_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_AV1_STD_SKIP_MODE_PRESENT_UNSET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_STD_SKIP_MODE_PRESENT_UNSET_BIT_KHR");
    if (VK_VIDEO_ENCODE_AV1_STD_PRIMARY_REF_FRAME_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_STD_PRIMARY_REF_FRAME_BIT_KHR");
    if (VK_VIDEO_ENCODE_AV1_STD_DELTA_Q_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_STD_DELTA_Q_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeAV1StdFlagsKHR(Printer &p, std::string name, VkVideoEncodeAV1StdFlagsKHR value) {
    if (static_cast<VkVideoEncodeAV1StdFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeAV1StdFlagBitsKHRGetStrings(static_cast<VkVideoEncodeAV1StdFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeAV1StdFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeAV1StdFlagBitsKHR value) {
    auto strings = VkVideoEncodeAV1StdFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeAV1SuperblockSizeFlagBitsKHRGetStrings(VkVideoEncodeAV1SuperblockSizeFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_64_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_64_BIT_KHR");
    if (VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_128_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_128_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeAV1SuperblockSizeFlagsKHR(Printer &p, std::string name, VkVideoEncodeAV1SuperblockSizeFlagsKHR value) {
    if (static_cast<VkVideoEncodeAV1SuperblockSizeFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeAV1SuperblockSizeFlagBitsKHRGetStrings(static_cast<VkVideoEncodeAV1SuperblockSizeFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeAV1SuperblockSizeFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeAV1SuperblockSizeFlagBitsKHR value) {
    auto strings = VkVideoEncodeAV1SuperblockSizeFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeCapabilityFlagBitsKHRGetStrings(VkVideoEncodeCapabilityFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_CAPABILITY_PRECEDING_EXTERNALLY_ENCODED_BYTES_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_CAPABILITY_PRECEDING_EXTERNALLY_ENCODED_BYTES_BIT_KHR");
    if (VK_VIDEO_ENCODE_CAPABILITY_INSUFFICIENT_BITSTREAM_BUFFER_RANGE_DETECTION_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_CAPABILITY_INSUFFICIENT_BITSTREAM_BUFFER_RANGE_DETECTION_BIT_KHR");
    if (VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR");
    if (VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeCapabilityFlagsKHR(Printer &p, std::string name, VkVideoEncodeCapabilityFlagsKHR value) {
    if (static_cast<VkVideoEncodeCapabilityFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeCapabilityFlagBitsKHRGetStrings(static_cast<VkVideoEncodeCapabilityFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeCapabilityFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeCapabilityFlagBitsKHR value) {
    auto strings = VkVideoEncodeCapabilityFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeContentFlagBitsKHRGetStrings(VkVideoEncodeContentFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("VIDEO_ENCODE_CONTENT_DEFAULT_KHR"); return strings; }
    if (VK_VIDEO_ENCODE_CONTENT_CAMERA_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_CONTENT_CAMERA_BIT_KHR");
    if (VK_VIDEO_ENCODE_CONTENT_DESKTOP_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_CONTENT_DESKTOP_BIT_KHR");
    if (VK_VIDEO_ENCODE_CONTENT_RENDERED_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_CONTENT_RENDERED_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeContentFlagsKHR(Printer &p, std::string name, VkVideoEncodeContentFlagsKHR value) {
    if (static_cast<VkVideoEncodeContentFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeContentFlagBitsKHRGetStrings(static_cast<VkVideoEncodeContentFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeContentFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeContentFlagBitsKHR value) {
    auto strings = VkVideoEncodeContentFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeFeedbackFlagBitsKHRGetStrings(VkVideoEncodeFeedbackFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR");
    if (VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR");
    if (VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_HAS_OVERRIDES_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_FEEDBACK_BITSTREAM_HAS_OVERRIDES_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeFeedbackFlagsKHR(Printer &p, std::string name, VkVideoEncodeFeedbackFlagsKHR value) {
    if (static_cast<VkVideoEncodeFeedbackFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeFeedbackFlagBitsKHRGetStrings(static_cast<VkVideoEncodeFeedbackFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeFeedbackFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeFeedbackFlagBitsKHR value) {
    auto strings = VkVideoEncodeFeedbackFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeH264CapabilityFlagBitsKHRGetStrings(VkVideoEncodeH264CapabilityFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_HRD_COMPLIANCE_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_HRD_COMPLIANCE_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_CAPABILITY_MB_QP_DIFF_WRAPAROUND_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_CAPABILITY_MB_QP_DIFF_WRAPAROUND_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeH264CapabilityFlagsKHR(Printer &p, std::string name, VkVideoEncodeH264CapabilityFlagsKHR value) {
    if (static_cast<VkVideoEncodeH264CapabilityFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeH264CapabilityFlagBitsKHRGetStrings(static_cast<VkVideoEncodeH264CapabilityFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeH264CapabilityFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeH264CapabilityFlagBitsKHR value) {
    auto strings = VkVideoEncodeH264CapabilityFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeH264StdFlagBitsKHRGetStrings(VkVideoEncodeH264StdFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_H264_STD_SEPARATE_COLOR_PLANE_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_SEPARATE_COLOR_PLANE_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_QPPRIME_Y_ZERO_TRANSFORM_BYPASS_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_QPPRIME_Y_ZERO_TRANSFORM_BYPASS_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_SCALING_MATRIX_PRESENT_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_SCALING_MATRIX_PRESENT_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_CHROMA_QP_INDEX_OFFSET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_CHROMA_QP_INDEX_OFFSET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_SECOND_CHROMA_QP_INDEX_OFFSET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_SECOND_CHROMA_QP_INDEX_OFFSET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_PIC_INIT_QP_MINUS26_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_PIC_INIT_QP_MINUS26_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_WEIGHTED_PRED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_WEIGHTED_PRED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_WEIGHTED_BIPRED_IDC_EXPLICIT_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_WEIGHTED_BIPRED_IDC_EXPLICIT_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_WEIGHTED_BIPRED_IDC_IMPLICIT_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_WEIGHTED_BIPRED_IDC_IMPLICIT_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_TRANSFORM_8X8_MODE_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_TRANSFORM_8X8_MODE_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_DIRECT_SPATIAL_MV_PRED_FLAG_UNSET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_DIRECT_SPATIAL_MV_PRED_FLAG_UNSET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_ENTROPY_CODING_MODE_FLAG_UNSET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_ENTROPY_CODING_MODE_FLAG_UNSET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_ENTROPY_CODING_MODE_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_ENTROPY_CODING_MODE_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_DIRECT_8X8_INFERENCE_FLAG_UNSET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_DIRECT_8X8_INFERENCE_FLAG_UNSET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_CONSTRAINED_INTRA_PRED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_CONSTRAINED_INTRA_PRED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_DEBLOCKING_FILTER_DISABLED_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_DEBLOCKING_FILTER_DISABLED_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_DEBLOCKING_FILTER_ENABLED_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_DEBLOCKING_FILTER_ENABLED_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_DEBLOCKING_FILTER_PARTIAL_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_DEBLOCKING_FILTER_PARTIAL_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_SLICE_QP_DELTA_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_SLICE_QP_DELTA_BIT_KHR");
    if (VK_VIDEO_ENCODE_H264_STD_DIFFERENT_SLICE_QP_DELTA_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H264_STD_DIFFERENT_SLICE_QP_DELTA_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeH264StdFlagsKHR(Printer &p, std::string name, VkVideoEncodeH264StdFlagsKHR value) {
    if (static_cast<VkVideoEncodeH264StdFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeH264StdFlagBitsKHRGetStrings(static_cast<VkVideoEncodeH264StdFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeH264StdFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeH264StdFlagBitsKHR value) {
    auto strings = VkVideoEncodeH264StdFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeH265CapabilityFlagBitsKHRGetStrings(VkVideoEncodeH265CapabilityFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_HRD_COMPLIANCE_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_HRD_COMPLIANCE_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILES_PER_SLICE_SEGMENT_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILES_PER_SLICE_SEGMENT_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CAPABILITY_CU_QP_DIFF_WRAPAROUND_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CAPABILITY_CU_QP_DIFF_WRAPAROUND_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeH265CapabilityFlagsKHR(Printer &p, std::string name, VkVideoEncodeH265CapabilityFlagsKHR value) {
    if (static_cast<VkVideoEncodeH265CapabilityFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeH265CapabilityFlagBitsKHRGetStrings(static_cast<VkVideoEncodeH265CapabilityFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeH265CapabilityFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeH265CapabilityFlagBitsKHR value) {
    auto strings = VkVideoEncodeH265CapabilityFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeH265CtbSizeFlagBitsKHRGetStrings(VkVideoEncodeH265CtbSizeFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_H265_CTB_SIZE_16_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CTB_SIZE_16_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CTB_SIZE_32_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CTB_SIZE_32_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeH265CtbSizeFlagsKHR(Printer &p, std::string name, VkVideoEncodeH265CtbSizeFlagsKHR value) {
    if (static_cast<VkVideoEncodeH265CtbSizeFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeH265CtbSizeFlagBitsKHRGetStrings(static_cast<VkVideoEncodeH265CtbSizeFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeH265CtbSizeFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeH265CtbSizeFlagBitsKHR value) {
    auto strings = VkVideoEncodeH265CtbSizeFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeH265StdFlagBitsKHRGetStrings(VkVideoEncodeH265StdFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_H265_STD_SEPARATE_COLOR_PLANE_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_SEPARATE_COLOR_PLANE_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_SAMPLE_ADAPTIVE_OFFSET_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_SAMPLE_ADAPTIVE_OFFSET_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_SCALING_LIST_DATA_PRESENT_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_SCALING_LIST_DATA_PRESENT_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_PCM_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_PCM_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_SPS_TEMPORAL_MVP_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_SPS_TEMPORAL_MVP_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_INIT_QP_MINUS26_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_INIT_QP_MINUS26_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_WEIGHTED_PRED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_WEIGHTED_PRED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_WEIGHTED_BIPRED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_WEIGHTED_BIPRED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_LOG2_PARALLEL_MERGE_LEVEL_MINUS2_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_LOG2_PARALLEL_MERGE_LEVEL_MINUS2_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_SIGN_DATA_HIDING_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_SIGN_DATA_HIDING_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_TRANSFORM_SKIP_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_TRANSFORM_SKIP_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_TRANSFORM_SKIP_ENABLED_FLAG_UNSET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_TRANSFORM_SKIP_ENABLED_FLAG_UNSET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_PPS_SLICE_CHROMA_QP_OFFSETS_PRESENT_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_PPS_SLICE_CHROMA_QP_OFFSETS_PRESENT_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_TRANSQUANT_BYPASS_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_TRANSQUANT_BYPASS_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_CONSTRAINED_INTRA_PRED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_CONSTRAINED_INTRA_PRED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_ENTROPY_CODING_SYNC_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_ENTROPY_CODING_SYNC_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_DEBLOCKING_FILTER_OVERRIDE_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_DEBLOCKING_FILTER_OVERRIDE_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_DEPENDENT_SLICE_SEGMENTS_ENABLED_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_DEPENDENT_SLICE_SEGMENTS_ENABLED_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_DEPENDENT_SLICE_SEGMENT_FLAG_SET_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_DEPENDENT_SLICE_SEGMENT_FLAG_SET_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_SLICE_QP_DELTA_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_SLICE_QP_DELTA_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_STD_DIFFERENT_SLICE_QP_DELTA_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_STD_DIFFERENT_SLICE_QP_DELTA_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeH265StdFlagsKHR(Printer &p, std::string name, VkVideoEncodeH265StdFlagsKHR value) {
    if (static_cast<VkVideoEncodeH265StdFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeH265StdFlagBitsKHRGetStrings(static_cast<VkVideoEncodeH265StdFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeH265StdFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeH265StdFlagBitsKHR value) {
    auto strings = VkVideoEncodeH265StdFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeH265TransformBlockSizeFlagBitsKHRGetStrings(VkVideoEncodeH265TransformBlockSizeFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_4_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_4_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_8_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_8_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_16_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_16_BIT_KHR");
    if (VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_32_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_32_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeH265TransformBlockSizeFlagsKHR(Printer &p, std::string name, VkVideoEncodeH265TransformBlockSizeFlagsKHR value) {
    if (static_cast<VkVideoEncodeH265TransformBlockSizeFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeH265TransformBlockSizeFlagBitsKHRGetStrings(static_cast<VkVideoEncodeH265TransformBlockSizeFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeH265TransformBlockSizeFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeH265TransformBlockSizeFlagBitsKHR value) {
    auto strings = VkVideoEncodeH265TransformBlockSizeFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeRateControlModeFlagBitsKHRGetStrings(VkVideoEncodeRateControlModeFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR"); return strings; }
    if (VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR");
    if (VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR");
    if (VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeRateControlModeFlagsKHR(Printer &p, std::string name, VkVideoEncodeRateControlModeFlagsKHR value) {
    if (static_cast<VkVideoEncodeRateControlModeFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeRateControlModeFlagBitsKHRGetStrings(static_cast<VkVideoEncodeRateControlModeFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeRateControlModeFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeRateControlModeFlagBitsKHR value) {
    auto strings = VkVideoEncodeRateControlModeFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

std::vector<const char *> VkVideoEncodeUsageFlagBitsKHRGetStrings(VkVideoEncodeUsageFlagBitsKHR value) {
    std::vector<const char *> strings;
    if (value == 0) { strings.push_back("VIDEO_ENCODE_USAGE_DEFAULT_KHR"); return strings; }
    if (VK_VIDEO_ENCODE_USAGE_TRANSCODING_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_USAGE_TRANSCODING_BIT_KHR");
    if (VK_VIDEO_ENCODE_USAGE_STREAMING_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_USAGE_STREAMING_BIT_KHR");
    if (VK_VIDEO_ENCODE_USAGE_RECORDING_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_USAGE_RECORDING_BIT_KHR");
    if (VK_VIDEO_ENCODE_USAGE_CONFERENCING_BIT_KHR & value) strings.push_back("VIDEO_ENCODE_USAGE_CONFERENCING_BIT_KHR");
    return strings;
}
void DumpVkVideoEncodeUsageFlagsKHR(Printer &p, std::string name, VkVideoEncodeUsageFlagsKHR value) {
    if (static_cast<VkVideoEncodeUsageFlagBitsKHR>(value) == 0) {
        ArrayWrapper arr(p, name, 0);
        if (p.Type() != OutputType::json && p.Type() != OutputType::vkconfig_output)
            p.SetAsType().PrintString("None");
        return;
    }
    auto strings = VkVideoEncodeUsageFlagBitsKHRGetStrings(static_cast<VkVideoEncodeUsageFlagBitsKHR>(value));
    ArrayWrapper arr(p, name, strings.size());
    for(auto& str : strings){
        if (p.Type() == OutputType::json)
            p.SetAsType().PrintString(std::string("VK_") + str);
        else
            p.SetAsType().PrintString(str);
    }
}
void DumpVkVideoEncodeUsageFlagBitsKHR(Printer &p, std::string name, VkVideoEncodeUsageFlagBitsKHR value) {
    auto strings = VkVideoEncodeUsageFlagBitsKHRGetStrings(value);
    if (strings.size() > 0) {
        if (p.Type() == OutputType::json)
            p.PrintKeyString(name, std::string("VK_") + strings.at(0));
        else
            p.PrintKeyString(name, strings.at(0));
    }
}

void DumpVkComponentMapping(Printer &p, std::string name, const VkComponentMapping &obj) {
    ObjectWrapper object{p, name};
    DumpVkComponentSwizzle(p, "r", obj.r);
    DumpVkComponentSwizzle(p, "g", obj.g);
    DumpVkComponentSwizzle(p, "b", obj.b);
    DumpVkComponentSwizzle(p, "a", obj.a);
}
void DumpVkConformanceVersion(Printer &p, std::string name, const VkConformanceVersion &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(8);
    p.PrintKeyValue("major", static_cast<uint32_t>(obj.major));
    p.PrintKeyValue("minor", static_cast<uint32_t>(obj.minor));
    p.PrintKeyValue("subminor", static_cast<uint32_t>(obj.subminor));
    p.PrintKeyValue("patch", static_cast<uint32_t>(obj.patch));
}
void DumpVkDrmFormatModifierProperties2EXT(Printer &p, std::string name, const VkDrmFormatModifierProperties2EXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(27);
    p.PrintKeyValue("drmFormatModifier", obj.drmFormatModifier);
    p.PrintKeyValue("drmFormatModifierPlaneCount", obj.drmFormatModifierPlaneCount);
    p.SetOpenDetails();
    DumpVkFormatFeatureFlags2(p, "drmFormatModifierTilingFeatures", obj.drmFormatModifierTilingFeatures);
}
void DumpVkExtensionProperties(Printer &p, std::string name, const VkExtensionProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(18);
    p.PrintKeyString("extensionName", obj.extensionName);
    p.PrintKeyValue("specVersion", obj.specVersion);
}
void DumpVkExtent2D(Printer &p, std::string name, const VkExtent2D &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(6);
    p.PrintKeyValue("width", obj.width);
    p.PrintKeyValue("height", obj.height);
}
void DumpVkExtent3D(Printer &p, std::string name, const VkExtent3D &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(6);
    p.PrintKeyValue("width", obj.width);
    p.PrintKeyValue("height", obj.height);
    p.PrintKeyValue("depth", obj.depth);
}
void DumpVkFormatProperties(Printer &p, std::string name, const VkFormatProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetOpenDetails();
    DumpVkFormatFeatureFlags(p, "linearTilingFeatures", obj.linearTilingFeatures);
    p.SetOpenDetails();
    DumpVkFormatFeatureFlags(p, "optimalTilingFeatures", obj.optimalTilingFeatures);
    p.SetOpenDetails();
    DumpVkFormatFeatureFlags(p, "bufferFeatures", obj.bufferFeatures);
}
void DumpVkFormatProperties3(Printer &p, std::string name, const VkFormatProperties3 &obj) {
    ObjectWrapper object{p, name};
    p.SetOpenDetails();
    DumpVkFormatFeatureFlags2(p, "linearTilingFeatures", obj.linearTilingFeatures);
    p.SetOpenDetails();
    DumpVkFormatFeatureFlags2(p, "optimalTilingFeatures", obj.optimalTilingFeatures);
    p.SetOpenDetails();
    DumpVkFormatFeatureFlags2(p, "bufferFeatures", obj.bufferFeatures);
}
void DumpVkLayerProperties(Printer &p, std::string name, const VkLayerProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(21);
    p.PrintKeyString("layerName", obj.layerName);
    p.PrintKeyValue("specVersion", obj.specVersion);
    p.PrintKeyValue("implementationVersion", obj.implementationVersion);
    p.PrintKeyString("description", obj.description);
}
void DumpVkOffset2D(Printer &p, std::string name, const VkOffset2D &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(1);
    p.PrintKeyValue("x", obj.x);
    p.PrintKeyValue("y", obj.y);
}
void DumpVkPhysicalDevice16BitStorageFeatures(Printer &p, std::string name, const VkPhysicalDevice16BitStorageFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(34);
    p.PrintKeyBool("storageBuffer16BitAccess", static_cast<bool>(obj.storageBuffer16BitAccess));
    p.PrintKeyBool("uniformAndStorageBuffer16BitAccess", static_cast<bool>(obj.uniformAndStorageBuffer16BitAccess));
    p.PrintKeyBool("storagePushConstant16", static_cast<bool>(obj.storagePushConstant16));
    p.PrintKeyBool("storageInputOutput16", static_cast<bool>(obj.storageInputOutput16));
}
void DumpVkPhysicalDevice4444FormatsFeaturesEXT(Printer &p, std::string name, const VkPhysicalDevice4444FormatsFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(14);
    p.PrintKeyBool("formatA4R4G4B4", static_cast<bool>(obj.formatA4R4G4B4));
    p.PrintKeyBool("formatA4B4G4R4", static_cast<bool>(obj.formatA4B4G4R4));
}
void DumpVkPhysicalDevice8BitStorageFeatures(Printer &p, std::string name, const VkPhysicalDevice8BitStorageFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(33);
    p.PrintKeyBool("storageBuffer8BitAccess", static_cast<bool>(obj.storageBuffer8BitAccess));
    p.PrintKeyBool("uniformAndStorageBuffer8BitAccess", static_cast<bool>(obj.uniformAndStorageBuffer8BitAccess));
    p.PrintKeyBool("storagePushConstant8", static_cast<bool>(obj.storagePushConstant8));
}
void DumpVkPhysicalDeviceASTCDecodeFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceASTCDecodeFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(24);
    p.PrintKeyBool("decodeModeSharedExponent", static_cast<bool>(obj.decodeModeSharedExponent));
}
void DumpVkPhysicalDeviceAccelerationStructureFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceAccelerationStructureFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(53);
    p.PrintKeyBool("accelerationStructure", static_cast<bool>(obj.accelerationStructure));
    p.PrintKeyBool("accelerationStructureCaptureReplay", static_cast<bool>(obj.accelerationStructureCaptureReplay));
    p.PrintKeyBool("accelerationStructureIndirectBuild", static_cast<bool>(obj.accelerationStructureIndirectBuild));
    p.PrintKeyBool("accelerationStructureHostCommands", static_cast<bool>(obj.accelerationStructureHostCommands));
    p.PrintKeyBool("descriptorBindingAccelerationStructureUpdateAfterBind", static_cast<bool>(obj.descriptorBindingAccelerationStructureUpdateAfterBind));
}
void DumpVkPhysicalDeviceAccelerationStructurePropertiesKHR(Printer &p, std::string name, const VkPhysicalDeviceAccelerationStructurePropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(58);
    p.PrintKeyValue("maxGeometryCount", obj.maxGeometryCount);
    p.PrintKeyValue("maxInstanceCount", obj.maxInstanceCount);
    p.PrintKeyValue("maxPrimitiveCount", obj.maxPrimitiveCount);
    p.PrintKeyValue("maxPerStageDescriptorAccelerationStructures", obj.maxPerStageDescriptorAccelerationStructures);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindAccelerationStructures", obj.maxPerStageDescriptorUpdateAfterBindAccelerationStructures);
    p.PrintKeyValue("maxDescriptorSetAccelerationStructures", obj.maxDescriptorSetAccelerationStructures);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindAccelerationStructures", obj.maxDescriptorSetUpdateAfterBindAccelerationStructures);
    p.PrintKeyValue("minAccelerationStructureScratchOffsetAlignment", obj.minAccelerationStructureScratchOffsetAlignment);
}
void DumpVkPhysicalDeviceAddressBindingReportFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceAddressBindingReportFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("reportAddressBinding", static_cast<bool>(obj.reportAddressBinding));
}
void DumpVkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(34);
    p.PrintKeyBool("attachmentFeedbackLoopDynamicState", static_cast<bool>(obj.attachmentFeedbackLoopDynamicState));
}
void DumpVkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyBool("attachmentFeedbackLoopLayout", static_cast<bool>(obj.attachmentFeedbackLoopLayout));
}
void DumpVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(31);
    p.PrintKeyBool("advancedBlendCoherentOperations", static_cast<bool>(obj.advancedBlendCoherentOperations));
}
void DumpVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(37);
    p.PrintKeyValue("advancedBlendMaxColorAttachments", obj.advancedBlendMaxColorAttachments);
    p.PrintKeyBool("advancedBlendIndependentBlend", static_cast<bool>(obj.advancedBlendIndependentBlend));
    p.PrintKeyBool("advancedBlendNonPremultipliedSrcColor", static_cast<bool>(obj.advancedBlendNonPremultipliedSrcColor));
    p.PrintKeyBool("advancedBlendNonPremultipliedDstColor", static_cast<bool>(obj.advancedBlendNonPremultipliedDstColor));
    p.PrintKeyBool("advancedBlendCorrelatedOverlap", static_cast<bool>(obj.advancedBlendCorrelatedOverlap));
    p.PrintKeyBool("advancedBlendAllOperations", static_cast<bool>(obj.advancedBlendAllOperations));
}
void DumpVkPhysicalDeviceBorderColorSwizzleFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(27);
    p.PrintKeyBool("borderColorSwizzle", static_cast<bool>(obj.borderColorSwizzle));
    p.PrintKeyBool("borderColorSwizzleFromImage", static_cast<bool>(obj.borderColorSwizzleFromImage));
}
void DumpVkPhysicalDeviceBufferDeviceAddressFeatures(Printer &p, std::string name, const VkPhysicalDeviceBufferDeviceAddressFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    p.PrintKeyBool("bufferDeviceAddress", static_cast<bool>(obj.bufferDeviceAddress));
    p.PrintKeyBool("bufferDeviceAddressCaptureReplay", static_cast<bool>(obj.bufferDeviceAddressCaptureReplay));
    p.PrintKeyBool("bufferDeviceAddressMultiDevice", static_cast<bool>(obj.bufferDeviceAddressMultiDevice));
}
void DumpVkPhysicalDeviceBufferDeviceAddressFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    p.PrintKeyBool("bufferDeviceAddress", static_cast<bool>(obj.bufferDeviceAddress));
    p.PrintKeyBool("bufferDeviceAddressCaptureReplay", static_cast<bool>(obj.bufferDeviceAddressCaptureReplay));
    p.PrintKeyBool("bufferDeviceAddressMultiDevice", static_cast<bool>(obj.bufferDeviceAddressMultiDevice));
}
void DumpVkPhysicalDeviceColorWriteEnableFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceColorWriteEnableFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    p.PrintKeyBool("colorWriteEnable", static_cast<bool>(obj.colorWriteEnable));
}
void DumpVkPhysicalDeviceComputeShaderDerivativesFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyBool("computeDerivativeGroupQuads", static_cast<bool>(obj.computeDerivativeGroupQuads));
    p.PrintKeyBool("computeDerivativeGroupLinear", static_cast<bool>(obj.computeDerivativeGroupLinear));
}
void DumpVkPhysicalDeviceComputeShaderDerivativesPropertiesKHR(Printer &p, std::string name, const VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyBool("meshAndTaskShaderDerivatives", static_cast<bool>(obj.meshAndTaskShaderDerivatives));
}
void DumpVkPhysicalDeviceConditionalRenderingFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceConditionalRenderingFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(29);
    p.PrintKeyBool("conditionalRendering", static_cast<bool>(obj.conditionalRendering));
    p.PrintKeyBool("inheritedConditionalRendering", static_cast<bool>(obj.inheritedConditionalRendering));
}
void DumpVkPhysicalDeviceConservativeRasterizationPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceConservativeRasterizationPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(43);
    p.PrintKeyValue("primitiveOverestimationSize", obj.primitiveOverestimationSize);
    p.PrintKeyValue("maxExtraPrimitiveOverestimationSize", obj.maxExtraPrimitiveOverestimationSize);
    p.PrintKeyValue("extraPrimitiveOverestimationSizeGranularity", obj.extraPrimitiveOverestimationSizeGranularity);
    p.PrintKeyBool("primitiveUnderestimation", static_cast<bool>(obj.primitiveUnderestimation));
    p.PrintKeyBool("conservativePointAndLineRasterization", static_cast<bool>(obj.conservativePointAndLineRasterization));
    p.PrintKeyBool("degenerateTrianglesRasterized", static_cast<bool>(obj.degenerateTrianglesRasterized));
    p.PrintKeyBool("degenerateLinesRasterized", static_cast<bool>(obj.degenerateLinesRasterized));
    p.PrintKeyBool("fullyCoveredFragmentShaderInputVariable", static_cast<bool>(obj.fullyCoveredFragmentShaderInputVariable));
    p.PrintKeyBool("conservativeRasterizationPostDepthCoverage", static_cast<bool>(obj.conservativeRasterizationPostDepthCoverage));
}
void DumpVkPhysicalDeviceCooperativeMatrixFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceCooperativeMatrixFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(35);
    p.PrintKeyBool("cooperativeMatrix", static_cast<bool>(obj.cooperativeMatrix));
    p.PrintKeyBool("cooperativeMatrixRobustBufferAccess", static_cast<bool>(obj.cooperativeMatrixRobustBufferAccess));
}
void DumpVkPhysicalDeviceCooperativeMatrixPropertiesKHR(Printer &p, std::string name, const VkPhysicalDeviceCooperativeMatrixPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkShaderStageFlags(p, "cooperativeMatrixSupportedStages", obj.cooperativeMatrixSupportedStages);
}
void DumpVkPhysicalDeviceCustomBorderColorFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceCustomBorderColorFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(30);
    p.PrintKeyBool("customBorderColors", static_cast<bool>(obj.customBorderColors));
    p.PrintKeyBool("customBorderColorWithoutFormat", static_cast<bool>(obj.customBorderColorWithoutFormat));
}
void DumpVkPhysicalDeviceCustomBorderColorPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceCustomBorderColorPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyValue("maxCustomBorderColorSamplers", obj.maxCustomBorderColorSamplers);
}
void DumpVkPhysicalDeviceDepthBiasControlFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceDepthBiasControlFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(47);
    p.PrintKeyBool("depthBiasControl", static_cast<bool>(obj.depthBiasControl));
    p.PrintKeyBool("leastRepresentableValueForceUnormRepresentation", static_cast<bool>(obj.leastRepresentableValueForceUnormRepresentation));
    p.PrintKeyBool("floatRepresentation", static_cast<bool>(obj.floatRepresentation));
    p.PrintKeyBool("depthBiasExact", static_cast<bool>(obj.depthBiasExact));
}
void DumpVkPhysicalDeviceDepthClampControlFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceDepthClampControlFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("depthClampControl", static_cast<bool>(obj.depthClampControl));
}
void DumpVkPhysicalDeviceDepthClampZeroOneFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("depthClampZeroOne", static_cast<bool>(obj.depthClampZeroOne));
}
void DumpVkPhysicalDeviceDepthClipControlFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceDepthClipControlFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    p.PrintKeyBool("depthClipControl", static_cast<bool>(obj.depthClipControl));
}
void DumpVkPhysicalDeviceDepthClipEnableFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceDepthClipEnableFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(15);
    p.PrintKeyBool("depthClipEnable", static_cast<bool>(obj.depthClipEnable));
}
void DumpVkPhysicalDeviceDepthStencilResolveProperties(Printer &p, std::string name, const VkPhysicalDeviceDepthStencilResolveProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(22);
    DumpVkResolveModeFlags(p, "supportedDepthResolveModes", obj.supportedDepthResolveModes);
    DumpVkResolveModeFlags(p, "supportedStencilResolveModes", obj.supportedStencilResolveModes);
    p.PrintKeyBool("independentResolveNone", static_cast<bool>(obj.independentResolveNone));
    p.PrintKeyBool("independentResolve", static_cast<bool>(obj.independentResolve));
}
void DumpVkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(44);
    p.PrintKeyValue("combinedImageSamplerDensityMapDescriptorSize", obj.combinedImageSamplerDensityMapDescriptorSize);
}
void DumpVkPhysicalDeviceDescriptorBufferFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceDescriptorBufferFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(34);
    p.PrintKeyBool("descriptorBuffer", static_cast<bool>(obj.descriptorBuffer));
    p.PrintKeyBool("descriptorBufferCaptureReplay", static_cast<bool>(obj.descriptorBufferCaptureReplay));
    p.PrintKeyBool("descriptorBufferImageLayoutIgnored", static_cast<bool>(obj.descriptorBufferImageLayoutIgnored));
    p.PrintKeyBool("descriptorBufferPushDescriptors", static_cast<bool>(obj.descriptorBufferPushDescriptors));
}
void DumpVkPhysicalDeviceDescriptorBufferPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceDescriptorBufferPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(52);
    p.PrintKeyBool("combinedImageSamplerDescriptorSingleArray", static_cast<bool>(obj.combinedImageSamplerDescriptorSingleArray));
    p.PrintKeyBool("bufferlessPushDescriptors", static_cast<bool>(obj.bufferlessPushDescriptors));
    p.PrintKeyBool("allowSamplerImageViewPostSubmitCreation", static_cast<bool>(obj.allowSamplerImageViewPostSubmitCreation));
    p.PrintKeyValue("descriptorBufferOffsetAlignment", to_hex_str(p, obj.descriptorBufferOffsetAlignment));
    p.PrintKeyValue("maxDescriptorBufferBindings", obj.maxDescriptorBufferBindings);
    p.PrintKeyValue("maxResourceDescriptorBufferBindings", obj.maxResourceDescriptorBufferBindings);
    p.PrintKeyValue("maxSamplerDescriptorBufferBindings", obj.maxSamplerDescriptorBufferBindings);
    p.PrintKeyValue("maxEmbeddedImmutableSamplerBindings", obj.maxEmbeddedImmutableSamplerBindings);
    p.PrintKeyValue("maxEmbeddedImmutableSamplers", obj.maxEmbeddedImmutableSamplers);
    p.PrintKeyValue("bufferCaptureReplayDescriptorDataSize", obj.bufferCaptureReplayDescriptorDataSize);
    p.PrintKeyValue("imageCaptureReplayDescriptorDataSize", obj.imageCaptureReplayDescriptorDataSize);
    p.PrintKeyValue("imageViewCaptureReplayDescriptorDataSize", obj.imageViewCaptureReplayDescriptorDataSize);
    p.PrintKeyValue("samplerCaptureReplayDescriptorDataSize", obj.samplerCaptureReplayDescriptorDataSize);
    p.PrintKeyValue("accelerationStructureCaptureReplayDescriptorDataSize", obj.accelerationStructureCaptureReplayDescriptorDataSize);
    p.PrintKeyValue("samplerDescriptorSize", obj.samplerDescriptorSize);
    p.PrintKeyValue("combinedImageSamplerDescriptorSize", obj.combinedImageSamplerDescriptorSize);
    p.PrintKeyValue("sampledImageDescriptorSize", obj.sampledImageDescriptorSize);
    p.PrintKeyValue("storageImageDescriptorSize", obj.storageImageDescriptorSize);
    p.PrintKeyValue("uniformTexelBufferDescriptorSize", obj.uniformTexelBufferDescriptorSize);
    p.PrintKeyValue("robustUniformTexelBufferDescriptorSize", obj.robustUniformTexelBufferDescriptorSize);
    p.PrintKeyValue("storageTexelBufferDescriptorSize", obj.storageTexelBufferDescriptorSize);
    p.PrintKeyValue("robustStorageTexelBufferDescriptorSize", obj.robustStorageTexelBufferDescriptorSize);
    p.PrintKeyValue("uniformBufferDescriptorSize", obj.uniformBufferDescriptorSize);
    p.PrintKeyValue("robustUniformBufferDescriptorSize", obj.robustUniformBufferDescriptorSize);
    p.PrintKeyValue("storageBufferDescriptorSize", obj.storageBufferDescriptorSize);
    p.PrintKeyValue("robustStorageBufferDescriptorSize", obj.robustStorageBufferDescriptorSize);
    p.PrintKeyValue("inputAttachmentDescriptorSize", obj.inputAttachmentDescriptorSize);
    p.PrintKeyValue("accelerationStructureDescriptorSize", obj.accelerationStructureDescriptorSize);
    p.PrintKeyValue("maxSamplerDescriptorBufferRange", to_hex_str(p, obj.maxSamplerDescriptorBufferRange));
    p.PrintKeyValue("maxResourceDescriptorBufferRange", to_hex_str(p, obj.maxResourceDescriptorBufferRange));
    p.PrintKeyValue("samplerDescriptorBufferAddressSpaceSize", to_hex_str(p, obj.samplerDescriptorBufferAddressSpaceSize));
    p.PrintKeyValue("resourceDescriptorBufferAddressSpaceSize", to_hex_str(p, obj.resourceDescriptorBufferAddressSpaceSize));
    p.PrintKeyValue("descriptorBufferAddressSpaceSize", to_hex_str(p, obj.descriptorBufferAddressSpaceSize));
}
void DumpVkPhysicalDeviceDescriptorIndexingFeatures(Printer &p, std::string name, const VkPhysicalDeviceDescriptorIndexingFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(50);
    p.PrintKeyBool("shaderInputAttachmentArrayDynamicIndexing", static_cast<bool>(obj.shaderInputAttachmentArrayDynamicIndexing));
    p.PrintKeyBool("shaderUniformTexelBufferArrayDynamicIndexing", static_cast<bool>(obj.shaderUniformTexelBufferArrayDynamicIndexing));
    p.PrintKeyBool("shaderStorageTexelBufferArrayDynamicIndexing", static_cast<bool>(obj.shaderStorageTexelBufferArrayDynamicIndexing));
    p.PrintKeyBool("shaderUniformBufferArrayNonUniformIndexing", static_cast<bool>(obj.shaderUniformBufferArrayNonUniformIndexing));
    p.PrintKeyBool("shaderSampledImageArrayNonUniformIndexing", static_cast<bool>(obj.shaderSampledImageArrayNonUniformIndexing));
    p.PrintKeyBool("shaderStorageBufferArrayNonUniformIndexing", static_cast<bool>(obj.shaderStorageBufferArrayNonUniformIndexing));
    p.PrintKeyBool("shaderStorageImageArrayNonUniformIndexing", static_cast<bool>(obj.shaderStorageImageArrayNonUniformIndexing));
    p.PrintKeyBool("shaderInputAttachmentArrayNonUniformIndexing", static_cast<bool>(obj.shaderInputAttachmentArrayNonUniformIndexing));
    p.PrintKeyBool("shaderUniformTexelBufferArrayNonUniformIndexing", static_cast<bool>(obj.shaderUniformTexelBufferArrayNonUniformIndexing));
    p.PrintKeyBool("shaderStorageTexelBufferArrayNonUniformIndexing", static_cast<bool>(obj.shaderStorageTexelBufferArrayNonUniformIndexing));
    p.PrintKeyBool("descriptorBindingUniformBufferUpdateAfterBind", static_cast<bool>(obj.descriptorBindingUniformBufferUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingSampledImageUpdateAfterBind", static_cast<bool>(obj.descriptorBindingSampledImageUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingStorageImageUpdateAfterBind", static_cast<bool>(obj.descriptorBindingStorageImageUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingStorageBufferUpdateAfterBind", static_cast<bool>(obj.descriptorBindingStorageBufferUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingUniformTexelBufferUpdateAfterBind", static_cast<bool>(obj.descriptorBindingUniformTexelBufferUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingStorageTexelBufferUpdateAfterBind", static_cast<bool>(obj.descriptorBindingStorageTexelBufferUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingUpdateUnusedWhilePending", static_cast<bool>(obj.descriptorBindingUpdateUnusedWhilePending));
    p.PrintKeyBool("descriptorBindingPartiallyBound", static_cast<bool>(obj.descriptorBindingPartiallyBound));
    p.PrintKeyBool("descriptorBindingVariableDescriptorCount", static_cast<bool>(obj.descriptorBindingVariableDescriptorCount));
    p.PrintKeyBool("runtimeDescriptorArray", static_cast<bool>(obj.runtimeDescriptorArray));
}
void DumpVkPhysicalDeviceDescriptorIndexingProperties(Printer &p, std::string name, const VkPhysicalDeviceDescriptorIndexingProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(52);
    p.PrintKeyValue("maxUpdateAfterBindDescriptorsInAllPools", obj.maxUpdateAfterBindDescriptorsInAllPools);
    p.PrintKeyBool("shaderUniformBufferArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderUniformBufferArrayNonUniformIndexingNative));
    p.PrintKeyBool("shaderSampledImageArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderSampledImageArrayNonUniformIndexingNative));
    p.PrintKeyBool("shaderStorageBufferArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderStorageBufferArrayNonUniformIndexingNative));
    p.PrintKeyBool("shaderStorageImageArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderStorageImageArrayNonUniformIndexingNative));
    p.PrintKeyBool("shaderInputAttachmentArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderInputAttachmentArrayNonUniformIndexingNative));
    p.PrintKeyBool("robustBufferAccessUpdateAfterBind", static_cast<bool>(obj.robustBufferAccessUpdateAfterBind));
    p.PrintKeyBool("quadDivergentImplicitLod", static_cast<bool>(obj.quadDivergentImplicitLod));
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindSamplers", obj.maxPerStageDescriptorUpdateAfterBindSamplers);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindUniformBuffers", obj.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindStorageBuffers", obj.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindSampledImages", obj.maxPerStageDescriptorUpdateAfterBindSampledImages);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindStorageImages", obj.maxPerStageDescriptorUpdateAfterBindStorageImages);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindInputAttachments", obj.maxPerStageDescriptorUpdateAfterBindInputAttachments);
    p.PrintKeyValue("maxPerStageUpdateAfterBindResources", obj.maxPerStageUpdateAfterBindResources);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindSamplers", obj.maxDescriptorSetUpdateAfterBindSamplers);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindUniformBuffers", obj.maxDescriptorSetUpdateAfterBindUniformBuffers);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindUniformBuffersDynamic", obj.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindStorageBuffers", obj.maxDescriptorSetUpdateAfterBindStorageBuffers);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindStorageBuffersDynamic", obj.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindSampledImages", obj.maxDescriptorSetUpdateAfterBindSampledImages);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindStorageImages", obj.maxDescriptorSetUpdateAfterBindStorageImages);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindInputAttachments", obj.maxDescriptorSetUpdateAfterBindInputAttachments);
}
void DumpVkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(30);
    p.PrintKeyBool("deviceGeneratedCommands", static_cast<bool>(obj.deviceGeneratedCommands));
    p.PrintKeyBool("dynamicGeneratedPipelineLayout", static_cast<bool>(obj.dynamicGeneratedPipelineLayout));
}
void DumpVkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(45);
    p.PrintKeyValue("maxIndirectPipelineCount", obj.maxIndirectPipelineCount);
    p.PrintKeyValue("maxIndirectShaderObjectCount", obj.maxIndirectShaderObjectCount);
    p.PrintKeyValue("maxIndirectSequenceCount", obj.maxIndirectSequenceCount);
    p.PrintKeyValue("maxIndirectCommandsTokenCount", obj.maxIndirectCommandsTokenCount);
    p.PrintKeyValue("maxIndirectCommandsTokenOffset", obj.maxIndirectCommandsTokenOffset);
    p.PrintKeyValue("maxIndirectCommandsIndirectStride", obj.maxIndirectCommandsIndirectStride);
    DumpVkIndirectCommandsInputModeFlagsEXT(p, "supportedIndirectCommandsInputModes", obj.supportedIndirectCommandsInputModes);
    DumpVkShaderStageFlags(p, "supportedIndirectCommandsShaderStages", obj.supportedIndirectCommandsShaderStages);
    DumpVkShaderStageFlags(p, "supportedIndirectCommandsShaderStagesPipelineBinding", obj.supportedIndirectCommandsShaderStagesPipelineBinding);
    DumpVkShaderStageFlags(p, "supportedIndirectCommandsShaderStagesShaderBinding", obj.supportedIndirectCommandsShaderStagesShaderBinding);
    p.PrintKeyBool("deviceGeneratedCommandsTransformFeedback", static_cast<bool>(obj.deviceGeneratedCommandsTransformFeedback));
    p.PrintKeyBool("deviceGeneratedCommandsMultiDrawIndirectCount", static_cast<bool>(obj.deviceGeneratedCommandsMultiDrawIndirectCount));
}
void DumpVkPhysicalDeviceDeviceMemoryReportFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(18);
    p.PrintKeyBool("deviceMemoryReport", static_cast<bool>(obj.deviceMemoryReport));
}
void DumpVkPhysicalDeviceDiscardRectanglePropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceDiscardRectanglePropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyValue("maxDiscardRectangles", obj.maxDiscardRectangles);
}
void DumpVkPhysicalDeviceDriverProperties(Printer &p, std::string name, const VkPhysicalDeviceDriverProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(15);
    DumpVkDriverId(p, "driverID", obj.driverID);
    p.PrintKeyString("driverName", obj.driverName);
    p.PrintKeyString("driverInfo", obj.driverInfo);
    DumpVkConformanceVersion(p, "conformanceVersion", obj.conformanceVersion);
}
void DumpVkPhysicalDeviceDrmPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceDrmPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(12);
    p.PrintKeyBool("hasPrimary", static_cast<bool>(obj.hasPrimary));
    p.PrintKeyBool("hasRender", static_cast<bool>(obj.hasRender));
    p.PrintKeyValue("primaryMajor", obj.primaryMajor);
    p.PrintKeyValue("primaryMinor", obj.primaryMinor);
    p.PrintKeyValue("renderMajor", obj.renderMajor);
    p.PrintKeyValue("renderMinor", obj.renderMinor);
}
void DumpVkPhysicalDeviceDynamicRenderingFeatures(Printer &p, std::string name, const VkPhysicalDeviceDynamicRenderingFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    p.PrintKeyBool("dynamicRendering", static_cast<bool>(obj.dynamicRendering));
}
void DumpVkPhysicalDeviceDynamicRenderingLocalReadFeatures(Printer &p, std::string name, const VkPhysicalDeviceDynamicRenderingLocalReadFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(25);
    p.PrintKeyBool("dynamicRenderingLocalRead", static_cast<bool>(obj.dynamicRenderingLocalRead));
}
void DumpVkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(33);
    p.PrintKeyBool("dynamicRenderingUnusedAttachments", static_cast<bool>(obj.dynamicRenderingUnusedAttachments));
}
void DumpVkPhysicalDeviceExtendedDynamicState2FeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(39);
    p.PrintKeyBool("extendedDynamicState2", static_cast<bool>(obj.extendedDynamicState2));
    p.PrintKeyBool("extendedDynamicState2LogicOp", static_cast<bool>(obj.extendedDynamicState2LogicOp));
    p.PrintKeyBool("extendedDynamicState2PatchControlPoints", static_cast<bool>(obj.extendedDynamicState2PatchControlPoints));
}
void DumpVkPhysicalDeviceExtendedDynamicState3FeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(53);
    p.PrintKeyBool("extendedDynamicState3TessellationDomainOrigin", static_cast<bool>(obj.extendedDynamicState3TessellationDomainOrigin));
    p.PrintKeyBool("extendedDynamicState3DepthClampEnable", static_cast<bool>(obj.extendedDynamicState3DepthClampEnable));
    p.PrintKeyBool("extendedDynamicState3PolygonMode", static_cast<bool>(obj.extendedDynamicState3PolygonMode));
    p.PrintKeyBool("extendedDynamicState3RasterizationSamples", static_cast<bool>(obj.extendedDynamicState3RasterizationSamples));
    p.PrintKeyBool("extendedDynamicState3SampleMask", static_cast<bool>(obj.extendedDynamicState3SampleMask));
    p.PrintKeyBool("extendedDynamicState3AlphaToCoverageEnable", static_cast<bool>(obj.extendedDynamicState3AlphaToCoverageEnable));
    p.PrintKeyBool("extendedDynamicState3AlphaToOneEnable", static_cast<bool>(obj.extendedDynamicState3AlphaToOneEnable));
    p.PrintKeyBool("extendedDynamicState3LogicOpEnable", static_cast<bool>(obj.extendedDynamicState3LogicOpEnable));
    p.PrintKeyBool("extendedDynamicState3ColorBlendEnable", static_cast<bool>(obj.extendedDynamicState3ColorBlendEnable));
    p.PrintKeyBool("extendedDynamicState3ColorBlendEquation", static_cast<bool>(obj.extendedDynamicState3ColorBlendEquation));
    p.PrintKeyBool("extendedDynamicState3ColorWriteMask", static_cast<bool>(obj.extendedDynamicState3ColorWriteMask));
    p.PrintKeyBool("extendedDynamicState3RasterizationStream", static_cast<bool>(obj.extendedDynamicState3RasterizationStream));
    p.PrintKeyBool("extendedDynamicState3ConservativeRasterizationMode", static_cast<bool>(obj.extendedDynamicState3ConservativeRasterizationMode));
    p.PrintKeyBool("extendedDynamicState3ExtraPrimitiveOverestimationSize", static_cast<bool>(obj.extendedDynamicState3ExtraPrimitiveOverestimationSize));
    p.PrintKeyBool("extendedDynamicState3DepthClipEnable", static_cast<bool>(obj.extendedDynamicState3DepthClipEnable));
    p.PrintKeyBool("extendedDynamicState3SampleLocationsEnable", static_cast<bool>(obj.extendedDynamicState3SampleLocationsEnable));
    p.PrintKeyBool("extendedDynamicState3ColorBlendAdvanced", static_cast<bool>(obj.extendedDynamicState3ColorBlendAdvanced));
    p.PrintKeyBool("extendedDynamicState3ProvokingVertexMode", static_cast<bool>(obj.extendedDynamicState3ProvokingVertexMode));
    p.PrintKeyBool("extendedDynamicState3LineRasterizationMode", static_cast<bool>(obj.extendedDynamicState3LineRasterizationMode));
    p.PrintKeyBool("extendedDynamicState3LineStippleEnable", static_cast<bool>(obj.extendedDynamicState3LineStippleEnable));
    p.PrintKeyBool("extendedDynamicState3DepthClipNegativeOneToOne", static_cast<bool>(obj.extendedDynamicState3DepthClipNegativeOneToOne));
    p.PrintKeyBool("extendedDynamicState3ViewportWScalingEnable", static_cast<bool>(obj.extendedDynamicState3ViewportWScalingEnable));
    p.PrintKeyBool("extendedDynamicState3ViewportSwizzle", static_cast<bool>(obj.extendedDynamicState3ViewportSwizzle));
    p.PrintKeyBool("extendedDynamicState3CoverageToColorEnable", static_cast<bool>(obj.extendedDynamicState3CoverageToColorEnable));
    p.PrintKeyBool("extendedDynamicState3CoverageToColorLocation", static_cast<bool>(obj.extendedDynamicState3CoverageToColorLocation));
    p.PrintKeyBool("extendedDynamicState3CoverageModulationMode", static_cast<bool>(obj.extendedDynamicState3CoverageModulationMode));
    p.PrintKeyBool("extendedDynamicState3CoverageModulationTableEnable", static_cast<bool>(obj.extendedDynamicState3CoverageModulationTableEnable));
    p.PrintKeyBool("extendedDynamicState3CoverageModulationTable", static_cast<bool>(obj.extendedDynamicState3CoverageModulationTable));
    p.PrintKeyBool("extendedDynamicState3CoverageReductionMode", static_cast<bool>(obj.extendedDynamicState3CoverageReductionMode));
    p.PrintKeyBool("extendedDynamicState3RepresentativeFragmentTestEnable", static_cast<bool>(obj.extendedDynamicState3RepresentativeFragmentTestEnable));
    p.PrintKeyBool("extendedDynamicState3ShadingRateImageEnable", static_cast<bool>(obj.extendedDynamicState3ShadingRateImageEnable));
}
void DumpVkPhysicalDeviceExtendedDynamicState3PropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(36);
    p.PrintKeyBool("dynamicPrimitiveTopologyUnrestricted", static_cast<bool>(obj.dynamicPrimitiveTopologyUnrestricted));
}
void DumpVkPhysicalDeviceExtendedDynamicStateFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("extendedDynamicState", static_cast<bool>(obj.extendedDynamicState));
}
void DumpVkPhysicalDeviceExternalMemoryHostPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceExternalMemoryHostPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(31);
    p.PrintKeyValue("minImportedHostPointerAlignment", to_hex_str(p, obj.minImportedHostPointerAlignment));
}
void DumpVkPhysicalDeviceFaultFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceFaultFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyBool("deviceFault", static_cast<bool>(obj.deviceFault));
    p.PrintKeyBool("deviceFaultVendorBinary", static_cast<bool>(obj.deviceFaultVendorBinary));
}
void DumpVkPhysicalDeviceFeatures(Printer &p, std::string name, const VkPhysicalDeviceFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(39);
    p.PrintKeyBool("robustBufferAccess", static_cast<bool>(obj.robustBufferAccess));
    p.PrintKeyBool("fullDrawIndexUint32", static_cast<bool>(obj.fullDrawIndexUint32));
    p.PrintKeyBool("imageCubeArray", static_cast<bool>(obj.imageCubeArray));
    p.PrintKeyBool("independentBlend", static_cast<bool>(obj.independentBlend));
    p.PrintKeyBool("geometryShader", static_cast<bool>(obj.geometryShader));
    p.PrintKeyBool("tessellationShader", static_cast<bool>(obj.tessellationShader));
    p.PrintKeyBool("sampleRateShading", static_cast<bool>(obj.sampleRateShading));
    p.PrintKeyBool("dualSrcBlend", static_cast<bool>(obj.dualSrcBlend));
    p.PrintKeyBool("logicOp", static_cast<bool>(obj.logicOp));
    p.PrintKeyBool("multiDrawIndirect", static_cast<bool>(obj.multiDrawIndirect));
    p.PrintKeyBool("drawIndirectFirstInstance", static_cast<bool>(obj.drawIndirectFirstInstance));
    p.PrintKeyBool("depthClamp", static_cast<bool>(obj.depthClamp));
    p.PrintKeyBool("depthBiasClamp", static_cast<bool>(obj.depthBiasClamp));
    p.PrintKeyBool("fillModeNonSolid", static_cast<bool>(obj.fillModeNonSolid));
    p.PrintKeyBool("depthBounds", static_cast<bool>(obj.depthBounds));
    p.PrintKeyBool("wideLines", static_cast<bool>(obj.wideLines));
    p.PrintKeyBool("largePoints", static_cast<bool>(obj.largePoints));
    p.PrintKeyBool("alphaToOne", static_cast<bool>(obj.alphaToOne));
    p.PrintKeyBool("multiViewport", static_cast<bool>(obj.multiViewport));
    p.PrintKeyBool("samplerAnisotropy", static_cast<bool>(obj.samplerAnisotropy));
    p.PrintKeyBool("textureCompressionETC2", static_cast<bool>(obj.textureCompressionETC2));
    p.PrintKeyBool("textureCompressionASTC_LDR", static_cast<bool>(obj.textureCompressionASTC_LDR));
    p.PrintKeyBool("textureCompressionBC", static_cast<bool>(obj.textureCompressionBC));
    p.PrintKeyBool("occlusionQueryPrecise", static_cast<bool>(obj.occlusionQueryPrecise));
    p.PrintKeyBool("pipelineStatisticsQuery", static_cast<bool>(obj.pipelineStatisticsQuery));
    p.PrintKeyBool("vertexPipelineStoresAndAtomics", static_cast<bool>(obj.vertexPipelineStoresAndAtomics));
    p.PrintKeyBool("fragmentStoresAndAtomics", static_cast<bool>(obj.fragmentStoresAndAtomics));
    p.PrintKeyBool("shaderTessellationAndGeometryPointSize", static_cast<bool>(obj.shaderTessellationAndGeometryPointSize));
    p.PrintKeyBool("shaderImageGatherExtended", static_cast<bool>(obj.shaderImageGatherExtended));
    p.PrintKeyBool("shaderStorageImageExtendedFormats", static_cast<bool>(obj.shaderStorageImageExtendedFormats));
    p.PrintKeyBool("shaderStorageImageMultisample", static_cast<bool>(obj.shaderStorageImageMultisample));
    p.PrintKeyBool("shaderStorageImageReadWithoutFormat", static_cast<bool>(obj.shaderStorageImageReadWithoutFormat));
    p.PrintKeyBool("shaderStorageImageWriteWithoutFormat", static_cast<bool>(obj.shaderStorageImageWriteWithoutFormat));
    p.PrintKeyBool("shaderUniformBufferArrayDynamicIndexing", static_cast<bool>(obj.shaderUniformBufferArrayDynamicIndexing));
    p.PrintKeyBool("shaderSampledImageArrayDynamicIndexing", static_cast<bool>(obj.shaderSampledImageArrayDynamicIndexing));
    p.PrintKeyBool("shaderStorageBufferArrayDynamicIndexing", static_cast<bool>(obj.shaderStorageBufferArrayDynamicIndexing));
    p.PrintKeyBool("shaderStorageImageArrayDynamicIndexing", static_cast<bool>(obj.shaderStorageImageArrayDynamicIndexing));
    p.PrintKeyBool("shaderClipDistance", static_cast<bool>(obj.shaderClipDistance));
    p.PrintKeyBool("shaderCullDistance", static_cast<bool>(obj.shaderCullDistance));
    p.PrintKeyBool("shaderFloat64", static_cast<bool>(obj.shaderFloat64));
    p.PrintKeyBool("shaderInt64", static_cast<bool>(obj.shaderInt64));
    p.PrintKeyBool("shaderInt16", static_cast<bool>(obj.shaderInt16));
    p.PrintKeyBool("shaderResourceResidency", static_cast<bool>(obj.shaderResourceResidency));
    p.PrintKeyBool("shaderResourceMinLod", static_cast<bool>(obj.shaderResourceMinLod));
    p.PrintKeyBool("sparseBinding", static_cast<bool>(obj.sparseBinding));
    p.PrintKeyBool("sparseResidencyBuffer", static_cast<bool>(obj.sparseResidencyBuffer));
    p.PrintKeyBool("sparseResidencyImage2D", static_cast<bool>(obj.sparseResidencyImage2D));
    p.PrintKeyBool("sparseResidencyImage3D", static_cast<bool>(obj.sparseResidencyImage3D));
    p.PrintKeyBool("sparseResidency2Samples", static_cast<bool>(obj.sparseResidency2Samples));
    p.PrintKeyBool("sparseResidency4Samples", static_cast<bool>(obj.sparseResidency4Samples));
    p.PrintKeyBool("sparseResidency8Samples", static_cast<bool>(obj.sparseResidency8Samples));
    p.PrintKeyBool("sparseResidency16Samples", static_cast<bool>(obj.sparseResidency16Samples));
    p.PrintKeyBool("sparseResidencyAliased", static_cast<bool>(obj.sparseResidencyAliased));
    p.PrintKeyBool("variableMultisampleRate", static_cast<bool>(obj.variableMultisampleRate));
    p.PrintKeyBool("inheritedQueries", static_cast<bool>(obj.inheritedQueries));
}
void DumpVkPhysicalDeviceFloatControlsProperties(Printer &p, std::string name, const VkPhysicalDeviceFloatControlsProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(37);
    DumpVkShaderFloatControlsIndependence(p, "denormBehaviorIndependence", obj.denormBehaviorIndependence);
    DumpVkShaderFloatControlsIndependence(p, "roundingModeIndependence", obj.roundingModeIndependence);
    p.PrintKeyBool("shaderSignedZeroInfNanPreserveFloat16", static_cast<bool>(obj.shaderSignedZeroInfNanPreserveFloat16));
    p.PrintKeyBool("shaderSignedZeroInfNanPreserveFloat32", static_cast<bool>(obj.shaderSignedZeroInfNanPreserveFloat32));
    p.PrintKeyBool("shaderSignedZeroInfNanPreserveFloat64", static_cast<bool>(obj.shaderSignedZeroInfNanPreserveFloat64));
    p.PrintKeyBool("shaderDenormPreserveFloat16", static_cast<bool>(obj.shaderDenormPreserveFloat16));
    p.PrintKeyBool("shaderDenormPreserveFloat32", static_cast<bool>(obj.shaderDenormPreserveFloat32));
    p.PrintKeyBool("shaderDenormPreserveFloat64", static_cast<bool>(obj.shaderDenormPreserveFloat64));
    p.PrintKeyBool("shaderDenormFlushToZeroFloat16", static_cast<bool>(obj.shaderDenormFlushToZeroFloat16));
    p.PrintKeyBool("shaderDenormFlushToZeroFloat32", static_cast<bool>(obj.shaderDenormFlushToZeroFloat32));
    p.PrintKeyBool("shaderDenormFlushToZeroFloat64", static_cast<bool>(obj.shaderDenormFlushToZeroFloat64));
    p.PrintKeyBool("shaderRoundingModeRTEFloat16", static_cast<bool>(obj.shaderRoundingModeRTEFloat16));
    p.PrintKeyBool("shaderRoundingModeRTEFloat32", static_cast<bool>(obj.shaderRoundingModeRTEFloat32));
    p.PrintKeyBool("shaderRoundingModeRTEFloat64", static_cast<bool>(obj.shaderRoundingModeRTEFloat64));
    p.PrintKeyBool("shaderRoundingModeRTZFloat16", static_cast<bool>(obj.shaderRoundingModeRTZFloat16));
    p.PrintKeyBool("shaderRoundingModeRTZFloat32", static_cast<bool>(obj.shaderRoundingModeRTZFloat32));
    p.PrintKeyBool("shaderRoundingModeRTZFloat64", static_cast<bool>(obj.shaderRoundingModeRTZFloat64));
}
void DumpVkPhysicalDeviceFragmentDensityMap2FeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(26);
    p.PrintKeyBool("fragmentDensityMapDeferred", static_cast<bool>(obj.fragmentDensityMapDeferred));
}
void DumpVkPhysicalDeviceFragmentDensityMap2PropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceFragmentDensityMap2PropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(41);
    p.PrintKeyBool("subsampledLoads", static_cast<bool>(obj.subsampledLoads));
    p.PrintKeyBool("subsampledCoarseReconstructionEarlyAccess", static_cast<bool>(obj.subsampledCoarseReconstructionEarlyAccess));
    p.PrintKeyValue("maxSubsampledArrayLayers", obj.maxSubsampledArrayLayers);
    p.PrintKeyValue("maxDescriptorSetSubsampledSamplers", obj.maxDescriptorSetSubsampledSamplers);
}
void DumpVkPhysicalDeviceFragmentDensityMapFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceFragmentDensityMapFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(37);
    p.PrintKeyBool("fragmentDensityMap", static_cast<bool>(obj.fragmentDensityMap));
    p.PrintKeyBool("fragmentDensityMapDynamic", static_cast<bool>(obj.fragmentDensityMapDynamic));
    p.PrintKeyBool("fragmentDensityMapNonSubsampledImages", static_cast<bool>(obj.fragmentDensityMapNonSubsampledImages));
}
void DumpVkPhysicalDeviceFragmentDensityMapPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceFragmentDensityMapPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(26);
    DumpVkExtent2D(p, "minFragmentDensityTexelSize", obj.minFragmentDensityTexelSize);
    DumpVkExtent2D(p, "maxFragmentDensityTexelSize", obj.maxFragmentDensityTexelSize);
    p.PrintKeyBool("fragmentDensityInvocations", static_cast<bool>(obj.fragmentDensityInvocations));
}
void DumpVkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(25);
    p.PrintKeyBool("fragmentShaderBarycentric", static_cast<bool>(obj.fragmentShaderBarycentric));
}
void DumpVkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(Printer &p, std::string name, const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(47);
    p.PrintKeyBool("triStripVertexOrderIndependentOfProvokingVertex", static_cast<bool>(obj.triStripVertexOrderIndependentOfProvokingVertex));
}
void DumpVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(34);
    p.PrintKeyBool("fragmentShaderSampleInterlock", static_cast<bool>(obj.fragmentShaderSampleInterlock));
    p.PrintKeyBool("fragmentShaderPixelInterlock", static_cast<bool>(obj.fragmentShaderPixelInterlock));
    p.PrintKeyBool("fragmentShaderShadingRateInterlock", static_cast<bool>(obj.fragmentShaderShadingRateInterlock));
}
void DumpVkPhysicalDeviceFragmentShadingRateFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceFragmentShadingRateFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(29);
    p.PrintKeyBool("pipelineFragmentShadingRate", static_cast<bool>(obj.pipelineFragmentShadingRate));
    p.PrintKeyBool("primitiveFragmentShadingRate", static_cast<bool>(obj.primitiveFragmentShadingRate));
    p.PrintKeyBool("attachmentFragmentShadingRate", static_cast<bool>(obj.attachmentFragmentShadingRate));
}
void DumpVkPhysicalDeviceFragmentShadingRatePropertiesKHR(Printer &p, std::string name, const VkPhysicalDeviceFragmentShadingRatePropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(52);
    DumpVkExtent2D(p, "minFragmentShadingRateAttachmentTexelSize", obj.minFragmentShadingRateAttachmentTexelSize);
    DumpVkExtent2D(p, "maxFragmentShadingRateAttachmentTexelSize", obj.maxFragmentShadingRateAttachmentTexelSize);
    p.PrintKeyValue("maxFragmentShadingRateAttachmentTexelSizeAspectRatio", obj.maxFragmentShadingRateAttachmentTexelSizeAspectRatio);
    p.PrintKeyBool("primitiveFragmentShadingRateWithMultipleViewports", static_cast<bool>(obj.primitiveFragmentShadingRateWithMultipleViewports));
    p.PrintKeyBool("layeredShadingRateAttachments", static_cast<bool>(obj.layeredShadingRateAttachments));
    p.PrintKeyBool("fragmentShadingRateNonTrivialCombinerOps", static_cast<bool>(obj.fragmentShadingRateNonTrivialCombinerOps));
    DumpVkExtent2D(p, "maxFragmentSize", obj.maxFragmentSize);
    p.PrintKeyValue("maxFragmentSizeAspectRatio", obj.maxFragmentSizeAspectRatio);
    p.PrintKeyValue("maxFragmentShadingRateCoverageSamples", obj.maxFragmentShadingRateCoverageSamples);
    DumpVkSampleCountFlagBits(p, "maxFragmentShadingRateRasterizationSamples", obj.maxFragmentShadingRateRasterizationSamples);
    p.PrintKeyBool("fragmentShadingRateWithShaderDepthStencilWrites", static_cast<bool>(obj.fragmentShadingRateWithShaderDepthStencilWrites));
    p.PrintKeyBool("fragmentShadingRateWithSampleMask", static_cast<bool>(obj.fragmentShadingRateWithSampleMask));
    p.PrintKeyBool("fragmentShadingRateWithShaderSampleMask", static_cast<bool>(obj.fragmentShadingRateWithShaderSampleMask));
    p.PrintKeyBool("fragmentShadingRateWithConservativeRasterization", static_cast<bool>(obj.fragmentShadingRateWithConservativeRasterization));
    p.PrintKeyBool("fragmentShadingRateWithFragmentShaderInterlock", static_cast<bool>(obj.fragmentShadingRateWithFragmentShaderInterlock));
    p.PrintKeyBool("fragmentShadingRateWithCustomSampleLocations", static_cast<bool>(obj.fragmentShadingRateWithCustomSampleLocations));
    p.PrintKeyBool("fragmentShadingRateStrictMultiplyCombiner", static_cast<bool>(obj.fragmentShadingRateStrictMultiplyCombiner));
}
void DumpVkPhysicalDeviceFrameBoundaryFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceFrameBoundaryFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(13);
    p.PrintKeyBool("frameBoundary", static_cast<bool>(obj.frameBoundary));
}
void DumpVkPhysicalDeviceGlobalPriorityQueryFeatures(Printer &p, std::string name, const VkPhysicalDeviceGlobalPriorityQueryFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(19);
    p.PrintKeyBool("globalPriorityQuery", static_cast<bool>(obj.globalPriorityQuery));
}
void DumpVkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyBool("graphicsPipelineLibrary", static_cast<bool>(obj.graphicsPipelineLibrary));
}
void DumpVkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(57);
    p.PrintKeyBool("graphicsPipelineLibraryFastLinking", static_cast<bool>(obj.graphicsPipelineLibraryFastLinking));
    p.PrintKeyBool("graphicsPipelineLibraryIndependentInterpolationDecoration", static_cast<bool>(obj.graphicsPipelineLibraryIndependentInterpolationDecoration));
}
void DumpVkPhysicalDeviceHostImageCopyFeatures(Printer &p, std::string name, const VkPhysicalDeviceHostImageCopyFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(13);
    p.PrintKeyBool("hostImageCopy", static_cast<bool>(obj.hostImageCopy));
}
void DumpVkPhysicalDeviceHostImageCopyProperties(Printer &p, std::string name, const VkPhysicalDeviceHostImageCopyProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(35);
    p.PrintKeyValue("copySrcLayoutCount", obj.copySrcLayoutCount);
    if (obj.copySrcLayoutCount == 0 || obj.pCopySrcLayouts == nullptr) {
        p.PrintKeyString("pCopySrcLayouts", "NULL");
    } else {
        ArrayWrapper arr(p,"pCopySrcLayouts", obj.copySrcLayoutCount);
        for (uint32_t i = 0; i < obj.copySrcLayoutCount; i++) {
            DumpVkImageLayout(p, std::to_string(i), obj.pCopySrcLayouts[i]);
        }
    }
    p.PrintKeyValue("copyDstLayoutCount", obj.copyDstLayoutCount);
    if (obj.copyDstLayoutCount == 0 || obj.pCopyDstLayouts == nullptr) {
        p.PrintKeyString("pCopyDstLayouts", "NULL");
    } else {
        ArrayWrapper arr(p,"pCopyDstLayouts", obj.copyDstLayoutCount);
        for (uint32_t i = 0; i < obj.copyDstLayoutCount; i++) {
            DumpVkImageLayout(p, std::to_string(i), obj.pCopyDstLayouts[i]);
        }
    }
    p.PrintKeyValue("optimalTilingLayoutUUID", obj.optimalTilingLayoutUUID);
    p.PrintKeyBool("identicalMemoryTypeRequirements", static_cast<bool>(obj.identicalMemoryTypeRequirements));
}
void DumpVkPhysicalDeviceHostQueryResetFeatures(Printer &p, std::string name, const VkPhysicalDeviceHostQueryResetFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(14);
    p.PrintKeyBool("hostQueryReset", static_cast<bool>(obj.hostQueryReset));
}
void DumpVkPhysicalDeviceIDProperties(Printer &p, std::string name, const VkPhysicalDeviceIDProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(15);
    p.PrintKeyValue("deviceUUID", obj.deviceUUID);
    p.PrintKeyValue("driverUUID", obj.driverUUID);
    if (obj.deviceLUIDValid) { // special case
    p.PrintKeyValue("deviceLUID", obj.deviceLUID);
    }
    p.PrintKeyValue("deviceNodeMask", obj.deviceNodeMask);
    p.PrintKeyBool("deviceLUIDValid", static_cast<bool>(obj.deviceLUIDValid));
}
void DumpVkPhysicalDeviceImage2DViewOf3DFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("image2DViewOf3D", static_cast<bool>(obj.image2DViewOf3D));
    p.PrintKeyBool("sampler2DViewOf3D", static_cast<bool>(obj.sampler2DViewOf3D));
}
void DumpVkPhysicalDeviceImageCompressionControlFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceImageCompressionControlFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyBool("imageCompressionControl", static_cast<bool>(obj.imageCompressionControl));
}
void DumpVkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    p.PrintKeyBool("imageCompressionControlSwapchain", static_cast<bool>(obj.imageCompressionControlSwapchain));
}
void DumpVkPhysicalDeviceImageRobustnessFeatures(Printer &p, std::string name, const VkPhysicalDeviceImageRobustnessFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("robustImageAccess", static_cast<bool>(obj.robustImageAccess));
}
void DumpVkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(19);
    p.PrintKeyBool("imageSlicedViewOf3D", static_cast<bool>(obj.imageSlicedViewOf3D));
}
void DumpVkPhysicalDeviceImageViewMinLodFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceImageViewMinLodFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(6);
    p.PrintKeyBool("minLod", static_cast<bool>(obj.minLod));
}
void DumpVkPhysicalDeviceImagelessFramebufferFeatures(Printer &p, std::string name, const VkPhysicalDeviceImagelessFramebufferFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("imagelessFramebuffer", static_cast<bool>(obj.imagelessFramebuffer));
}
void DumpVkPhysicalDeviceIndexTypeUint8Features(Printer &p, std::string name, const VkPhysicalDeviceIndexTypeUint8Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(14);
    p.PrintKeyBool("indexTypeUint8", static_cast<bool>(obj.indexTypeUint8));
}
void DumpVkPhysicalDeviceInlineUniformBlockFeatures(Printer &p, std::string name, const VkPhysicalDeviceInlineUniformBlockFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(50);
    p.PrintKeyBool("inlineUniformBlock", static_cast<bool>(obj.inlineUniformBlock));
    p.PrintKeyBool("descriptorBindingInlineUniformBlockUpdateAfterBind", static_cast<bool>(obj.descriptorBindingInlineUniformBlockUpdateAfterBind));
}
void DumpVkPhysicalDeviceInlineUniformBlockProperties(Printer &p, std::string name, const VkPhysicalDeviceInlineUniformBlockProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(55);
    p.PrintKeyValue("maxInlineUniformBlockSize", obj.maxInlineUniformBlockSize);
    p.PrintKeyValue("maxPerStageDescriptorInlineUniformBlocks", obj.maxPerStageDescriptorInlineUniformBlocks);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks", obj.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks);
    p.PrintKeyValue("maxDescriptorSetInlineUniformBlocks", obj.maxDescriptorSetInlineUniformBlocks);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindInlineUniformBlocks", obj.maxDescriptorSetUpdateAfterBindInlineUniformBlocks);
}
void DumpVkPhysicalDeviceLayeredApiPropertiesKHR(Printer &p, std::string name, const VkPhysicalDeviceLayeredApiPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(15);
    p.PrintKeyValue("vendorID", obj.vendorID);
    p.PrintKeyValue("deviceID", obj.deviceID);
    DumpVkPhysicalDeviceLayeredApiKHR(p, "layeredAPI", obj.layeredAPI);
    p.PrintKeyString("deviceName", obj.deviceName);
}
void DumpVkPhysicalDeviceLayeredApiPropertiesListKHR(Printer &p, std::string name, const VkPhysicalDeviceLayeredApiPropertiesListKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(29);
    p.PrintKeyValue("layeredApiCount", obj.layeredApiCount);
    if (obj.layeredApiCount == 0 || obj.pLayeredApis == nullptr) {
        p.PrintKeyString("pLayeredApis", "NULL");
    } else {
        ArrayWrapper arr(p,"pLayeredApis", obj.layeredApiCount);
        for (uint32_t i = 0; i < obj.layeredApiCount; i++) {
            DumpVkPhysicalDeviceLayeredApiPropertiesKHR(p, std::to_string(i), obj.pLayeredApis[i]);
        }
    }
}
void DumpVkPhysicalDeviceLegacyDitheringFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceLegacyDitheringFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(15);
    p.PrintKeyBool("legacyDithering", static_cast<bool>(obj.legacyDithering));
}
void DumpVkPhysicalDeviceLegacyVertexAttributesFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(22);
    p.PrintKeyBool("legacyVertexAttributes", static_cast<bool>(obj.legacyVertexAttributes));
}
void DumpVkPhysicalDeviceLegacyVertexAttributesPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(26);
    p.PrintKeyBool("nativeUnalignedPerformance", static_cast<bool>(obj.nativeUnalignedPerformance));
}
void DumpVkPhysicalDeviceLimits(Printer &p, std::string name, const VkPhysicalDeviceLimits &obj) {
    if (p.Type() == OutputType::json)
        p.ObjectStart("limits");
    else
        p.SetSubHeader().ObjectStart(name);
    p.SetMinKeyWidth(47);
    p.PrintKeyValue("maxImageDimension1D", obj.maxImageDimension1D);
    p.PrintKeyValue("maxImageDimension2D", obj.maxImageDimension2D);
    p.PrintKeyValue("maxImageDimension3D", obj.maxImageDimension3D);
    p.PrintKeyValue("maxImageDimensionCube", obj.maxImageDimensionCube);
    p.PrintKeyValue("maxImageArrayLayers", obj.maxImageArrayLayers);
    p.PrintKeyValue("maxTexelBufferElements", obj.maxTexelBufferElements);
    p.PrintKeyValue("maxUniformBufferRange", obj.maxUniformBufferRange);
    p.PrintKeyValue("maxStorageBufferRange", obj.maxStorageBufferRange);
    p.PrintKeyValue("maxPushConstantsSize", obj.maxPushConstantsSize);
    p.PrintKeyValue("maxMemoryAllocationCount", obj.maxMemoryAllocationCount);
    p.PrintKeyValue("maxSamplerAllocationCount", obj.maxSamplerAllocationCount);
    p.PrintKeyValue("bufferImageGranularity", to_hex_str(p, obj.bufferImageGranularity));
    p.PrintKeyValue("sparseAddressSpaceSize", to_hex_str(p, obj.sparseAddressSpaceSize));
    p.PrintKeyValue("maxBoundDescriptorSets", obj.maxBoundDescriptorSets);
    p.PrintKeyValue("maxPerStageDescriptorSamplers", obj.maxPerStageDescriptorSamplers);
    p.PrintKeyValue("maxPerStageDescriptorUniformBuffers", obj.maxPerStageDescriptorUniformBuffers);
    p.PrintKeyValue("maxPerStageDescriptorStorageBuffers", obj.maxPerStageDescriptorStorageBuffers);
    p.PrintKeyValue("maxPerStageDescriptorSampledImages", obj.maxPerStageDescriptorSampledImages);
    p.PrintKeyValue("maxPerStageDescriptorStorageImages", obj.maxPerStageDescriptorStorageImages);
    p.PrintKeyValue("maxPerStageDescriptorInputAttachments", obj.maxPerStageDescriptorInputAttachments);
    p.PrintKeyValue("maxPerStageResources", obj.maxPerStageResources);
    p.PrintKeyValue("maxDescriptorSetSamplers", obj.maxDescriptorSetSamplers);
    p.PrintKeyValue("maxDescriptorSetUniformBuffers", obj.maxDescriptorSetUniformBuffers);
    p.PrintKeyValue("maxDescriptorSetUniformBuffersDynamic", obj.maxDescriptorSetUniformBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetStorageBuffers", obj.maxDescriptorSetStorageBuffers);
    p.PrintKeyValue("maxDescriptorSetStorageBuffersDynamic", obj.maxDescriptorSetStorageBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetSampledImages", obj.maxDescriptorSetSampledImages);
    p.PrintKeyValue("maxDescriptorSetStorageImages", obj.maxDescriptorSetStorageImages);
    p.PrintKeyValue("maxDescriptorSetInputAttachments", obj.maxDescriptorSetInputAttachments);
    p.PrintKeyValue("maxVertexInputAttributes", obj.maxVertexInputAttributes);
    p.PrintKeyValue("maxVertexInputBindings", obj.maxVertexInputBindings);
    p.PrintKeyValue("maxVertexInputAttributeOffset", obj.maxVertexInputAttributeOffset);
    p.PrintKeyValue("maxVertexInputBindingStride", obj.maxVertexInputBindingStride);
    p.PrintKeyValue("maxVertexOutputComponents", obj.maxVertexOutputComponents);
    p.PrintKeyValue("maxTessellationGenerationLevel", obj.maxTessellationGenerationLevel);
    p.PrintKeyValue("maxTessellationPatchSize", obj.maxTessellationPatchSize);
    p.PrintKeyValue("maxTessellationControlPerVertexInputComponents", obj.maxTessellationControlPerVertexInputComponents);
    p.PrintKeyValue("maxTessellationControlPerVertexOutputComponents", obj.maxTessellationControlPerVertexOutputComponents);
    p.PrintKeyValue("maxTessellationControlPerPatchOutputComponents", obj.maxTessellationControlPerPatchOutputComponents);
    p.PrintKeyValue("maxTessellationControlTotalOutputComponents", obj.maxTessellationControlTotalOutputComponents);
    p.PrintKeyValue("maxTessellationEvaluationInputComponents", obj.maxTessellationEvaluationInputComponents);
    p.PrintKeyValue("maxTessellationEvaluationOutputComponents", obj.maxTessellationEvaluationOutputComponents);
    p.PrintKeyValue("maxGeometryShaderInvocations", obj.maxGeometryShaderInvocations);
    p.PrintKeyValue("maxGeometryInputComponents", obj.maxGeometryInputComponents);
    p.PrintKeyValue("maxGeometryOutputComponents", obj.maxGeometryOutputComponents);
    p.PrintKeyValue("maxGeometryOutputVertices", obj.maxGeometryOutputVertices);
    p.PrintKeyValue("maxGeometryTotalOutputComponents", obj.maxGeometryTotalOutputComponents);
    p.PrintKeyValue("maxFragmentInputComponents", obj.maxFragmentInputComponents);
    p.PrintKeyValue("maxFragmentOutputAttachments", obj.maxFragmentOutputAttachments);
    p.PrintKeyValue("maxFragmentDualSrcAttachments", obj.maxFragmentDualSrcAttachments);
    p.PrintKeyValue("maxFragmentCombinedOutputResources", obj.maxFragmentCombinedOutputResources);
    p.PrintKeyValue("maxComputeSharedMemorySize", obj.maxComputeSharedMemorySize);
    {
        ArrayWrapper arr(p,"maxComputeWorkGroupCount", 3);
        for (uint32_t i = 0; i < 3; i++) { p.PrintElement(obj.maxComputeWorkGroupCount[i]); }
    }
    p.PrintKeyValue("maxComputeWorkGroupInvocations", obj.maxComputeWorkGroupInvocations);
    {
        ArrayWrapper arr(p,"maxComputeWorkGroupSize", 3);
        for (uint32_t i = 0; i < 3; i++) { p.PrintElement(obj.maxComputeWorkGroupSize[i]); }
    }
    p.PrintKeyValue("subPixelPrecisionBits", obj.subPixelPrecisionBits);
    p.PrintKeyValue("subTexelPrecisionBits", obj.subTexelPrecisionBits);
    p.PrintKeyValue("mipmapPrecisionBits", obj.mipmapPrecisionBits);
    p.PrintKeyValue("maxDrawIndexedIndexValue", obj.maxDrawIndexedIndexValue);
    p.PrintKeyValue("maxDrawIndirectCount", obj.maxDrawIndirectCount);
    p.PrintKeyValue("maxSamplerLodBias", obj.maxSamplerLodBias);
    p.PrintKeyValue("maxSamplerAnisotropy", obj.maxSamplerAnisotropy);
    p.PrintKeyValue("maxViewports", obj.maxViewports);
    {
        ArrayWrapper arr(p,"maxViewportDimensions", 2);
        for (uint32_t i = 0; i < 2; i++) { p.PrintElement(obj.maxViewportDimensions[i]); }
    }
    {
        ArrayWrapper arr(p,"viewportBoundsRange", 2);
        for (uint32_t i = 0; i < 2; i++) { p.PrintElement(obj.viewportBoundsRange[i]); }
    }
    p.PrintKeyValue("viewportSubPixelBits", obj.viewportSubPixelBits);
    p.PrintKeyValue("minMemoryMapAlignment", obj.minMemoryMapAlignment);
    p.PrintKeyValue("minTexelBufferOffsetAlignment", to_hex_str(p, obj.minTexelBufferOffsetAlignment));
    p.PrintKeyValue("minUniformBufferOffsetAlignment", to_hex_str(p, obj.minUniformBufferOffsetAlignment));
    p.PrintKeyValue("minStorageBufferOffsetAlignment", to_hex_str(p, obj.minStorageBufferOffsetAlignment));
    p.PrintKeyValue("minTexelOffset", obj.minTexelOffset);
    p.PrintKeyValue("maxTexelOffset", obj.maxTexelOffset);
    p.PrintKeyValue("minTexelGatherOffset", obj.minTexelGatherOffset);
    p.PrintKeyValue("maxTexelGatherOffset", obj.maxTexelGatherOffset);
    p.PrintKeyValue("minInterpolationOffset", obj.minInterpolationOffset);
    p.PrintKeyValue("maxInterpolationOffset", obj.maxInterpolationOffset);
    p.PrintKeyValue("subPixelInterpolationOffsetBits", obj.subPixelInterpolationOffsetBits);
    p.PrintKeyValue("maxFramebufferWidth", obj.maxFramebufferWidth);
    p.PrintKeyValue("maxFramebufferHeight", obj.maxFramebufferHeight);
    p.PrintKeyValue("maxFramebufferLayers", obj.maxFramebufferLayers);
    DumpVkSampleCountFlags(p, "framebufferColorSampleCounts", obj.framebufferColorSampleCounts);
    DumpVkSampleCountFlags(p, "framebufferDepthSampleCounts", obj.framebufferDepthSampleCounts);
    DumpVkSampleCountFlags(p, "framebufferStencilSampleCounts", obj.framebufferStencilSampleCounts);
    DumpVkSampleCountFlags(p, "framebufferNoAttachmentsSampleCounts", obj.framebufferNoAttachmentsSampleCounts);
    p.PrintKeyValue("maxColorAttachments", obj.maxColorAttachments);
    DumpVkSampleCountFlags(p, "sampledImageColorSampleCounts", obj.sampledImageColorSampleCounts);
    DumpVkSampleCountFlags(p, "sampledImageIntegerSampleCounts", obj.sampledImageIntegerSampleCounts);
    DumpVkSampleCountFlags(p, "sampledImageDepthSampleCounts", obj.sampledImageDepthSampleCounts);
    DumpVkSampleCountFlags(p, "sampledImageStencilSampleCounts", obj.sampledImageStencilSampleCounts);
    DumpVkSampleCountFlags(p, "storageImageSampleCounts", obj.storageImageSampleCounts);
    p.PrintKeyValue("maxSampleMaskWords", obj.maxSampleMaskWords);
    p.PrintKeyBool("timestampComputeAndGraphics", static_cast<bool>(obj.timestampComputeAndGraphics));
    p.PrintKeyValue("timestampPeriod", obj.timestampPeriod);
    p.PrintKeyValue("maxClipDistances", obj.maxClipDistances);
    p.PrintKeyValue("maxCullDistances", obj.maxCullDistances);
    p.PrintKeyValue("maxCombinedClipAndCullDistances", obj.maxCombinedClipAndCullDistances);
    p.PrintKeyValue("discreteQueuePriorities", obj.discreteQueuePriorities);
    {
        ArrayWrapper arr(p,"pointSizeRange", 2);
        for (uint32_t i = 0; i < 2; i++) { p.PrintElement(obj.pointSizeRange[i]); }
    }
    {
        ArrayWrapper arr(p,"lineWidthRange", 2);
        for (uint32_t i = 0; i < 2; i++) { p.PrintElement(obj.lineWidthRange[i]); }
    }
    p.PrintKeyValue("pointSizeGranularity", obj.pointSizeGranularity);
    p.PrintKeyValue("lineWidthGranularity", obj.lineWidthGranularity);
    p.PrintKeyBool("strictLines", static_cast<bool>(obj.strictLines));
    p.PrintKeyBool("standardSampleLocations", static_cast<bool>(obj.standardSampleLocations));
    p.PrintKeyValue("optimalBufferCopyOffsetAlignment", to_hex_str(p, obj.optimalBufferCopyOffsetAlignment));
    p.PrintKeyValue("optimalBufferCopyRowPitchAlignment", to_hex_str(p, obj.optimalBufferCopyRowPitchAlignment));
    p.PrintKeyValue("nonCoherentAtomSize", to_hex_str(p, obj.nonCoherentAtomSize));
    p.ObjectEnd();
}
void DumpVkPhysicalDeviceLineRasterizationFeatures(Printer &p, std::string name, const VkPhysicalDeviceLineRasterizationFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(24);
    p.PrintKeyBool("rectangularLines", static_cast<bool>(obj.rectangularLines));
    p.PrintKeyBool("bresenhamLines", static_cast<bool>(obj.bresenhamLines));
    p.PrintKeyBool("smoothLines", static_cast<bool>(obj.smoothLines));
    p.PrintKeyBool("stippledRectangularLines", static_cast<bool>(obj.stippledRectangularLines));
    p.PrintKeyBool("stippledBresenhamLines", static_cast<bool>(obj.stippledBresenhamLines));
    p.PrintKeyBool("stippledSmoothLines", static_cast<bool>(obj.stippledSmoothLines));
}
void DumpVkPhysicalDeviceLineRasterizationProperties(Printer &p, std::string name, const VkPhysicalDeviceLineRasterizationProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(25);
    p.PrintKeyValue("lineSubPixelPrecisionBits", obj.lineSubPixelPrecisionBits);
}
void DumpVkPhysicalDeviceMaintenance3Properties(Printer &p, std::string name, const VkPhysicalDeviceMaintenance3Properties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyValue("maxPerSetDescriptors", obj.maxPerSetDescriptors);
    p.PrintKeyValue("maxMemoryAllocationSize", to_hex_str(p, obj.maxMemoryAllocationSize));
}
void DumpVkPhysicalDeviceMaintenance4Features(Printer &p, std::string name, const VkPhysicalDeviceMaintenance4Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(12);
    p.PrintKeyBool("maintenance4", static_cast<bool>(obj.maintenance4));
}
void DumpVkPhysicalDeviceMaintenance4Properties(Printer &p, std::string name, const VkPhysicalDeviceMaintenance4Properties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(13);
    p.PrintKeyValue("maxBufferSize", to_hex_str(p, obj.maxBufferSize));
}
void DumpVkPhysicalDeviceMaintenance5Features(Printer &p, std::string name, const VkPhysicalDeviceMaintenance5Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(12);
    p.PrintKeyBool("maintenance5", static_cast<bool>(obj.maintenance5));
}
void DumpVkPhysicalDeviceMaintenance5Properties(Printer &p, std::string name, const VkPhysicalDeviceMaintenance5Properties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(51);
    p.PrintKeyBool("earlyFragmentMultisampleCoverageAfterSampleCounting", static_cast<bool>(obj.earlyFragmentMultisampleCoverageAfterSampleCounting));
    p.PrintKeyBool("earlyFragmentSampleMaskTestBeforeSampleCounting", static_cast<bool>(obj.earlyFragmentSampleMaskTestBeforeSampleCounting));
    p.PrintKeyBool("depthStencilSwizzleOneSupport", static_cast<bool>(obj.depthStencilSwizzleOneSupport));
    p.PrintKeyBool("polygonModePointSize", static_cast<bool>(obj.polygonModePointSize));
    p.PrintKeyBool("nonStrictSinglePixelWideLinesUseParallelogram", static_cast<bool>(obj.nonStrictSinglePixelWideLinesUseParallelogram));
    p.PrintKeyBool("nonStrictWideLinesUseParallelogram", static_cast<bool>(obj.nonStrictWideLinesUseParallelogram));
}
void DumpVkPhysicalDeviceMaintenance6Features(Printer &p, std::string name, const VkPhysicalDeviceMaintenance6Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(12);
    p.PrintKeyBool("maintenance6", static_cast<bool>(obj.maintenance6));
}
void DumpVkPhysicalDeviceMaintenance6Properties(Printer &p, std::string name, const VkPhysicalDeviceMaintenance6Properties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(38);
    p.PrintKeyBool("blockTexelViewCompatibleMultipleLayers", static_cast<bool>(obj.blockTexelViewCompatibleMultipleLayers));
    p.PrintKeyValue("maxCombinedImageSamplerDescriptorCount", obj.maxCombinedImageSamplerDescriptorCount);
    p.PrintKeyBool("fragmentShadingRateClampCombinerInputs", static_cast<bool>(obj.fragmentShadingRateClampCombinerInputs));
}
void DumpVkPhysicalDeviceMaintenance7FeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceMaintenance7FeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(12);
    p.PrintKeyBool("maintenance7", static_cast<bool>(obj.maintenance7));
}
void DumpVkPhysicalDeviceMaintenance7PropertiesKHR(Printer &p, std::string name, const VkPhysicalDeviceMaintenance7PropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(57);
    p.PrintKeyBool("robustFragmentShadingRateAttachmentAccess", static_cast<bool>(obj.robustFragmentShadingRateAttachmentAccess));
    p.PrintKeyBool("separateDepthStencilAttachmentAccess", static_cast<bool>(obj.separateDepthStencilAttachmentAccess));
    p.PrintKeyValue("maxDescriptorSetTotalUniformBuffersDynamic", obj.maxDescriptorSetTotalUniformBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetTotalStorageBuffersDynamic", obj.maxDescriptorSetTotalStorageBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetTotalBuffersDynamic", obj.maxDescriptorSetTotalBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic", obj.maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic", obj.maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindTotalBuffersDynamic", obj.maxDescriptorSetUpdateAfterBindTotalBuffersDynamic);
}
void DumpVkPhysicalDeviceMaintenance8FeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceMaintenance8FeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(12);
    p.PrintKeyBool("maintenance8", static_cast<bool>(obj.maintenance8));
}
void DumpVkPhysicalDeviceMapMemoryPlacedFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("memoryMapPlaced", static_cast<bool>(obj.memoryMapPlaced));
    p.PrintKeyBool("memoryMapRangePlaced", static_cast<bool>(obj.memoryMapRangePlaced));
    p.PrintKeyBool("memoryUnmapReserve", static_cast<bool>(obj.memoryUnmapReserve));
}
void DumpVkPhysicalDeviceMapMemoryPlacedPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceMapMemoryPlacedPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(27);
    p.PrintKeyValue("minPlacedMemoryMapAlignment", to_hex_str(p, obj.minPlacedMemoryMapAlignment));
}
void DumpVkPhysicalDeviceMemoryBudgetPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceMemoryBudgetPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(14);
    {
        ArrayWrapper arr(p,"heapBudget", 16);
        for (uint32_t i = 0; i < 16; i++) { p.PrintElement(obj.heapBudget[i]); }
    }
    {
        ArrayWrapper arr(p,"heapUsage", 16);
        for (uint32_t i = 0; i < 16; i++) { p.PrintElement(obj.heapUsage[i]); }
    }
}
void DumpVkPhysicalDeviceMemoryPriorityFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceMemoryPriorityFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(14);
    p.PrintKeyBool("memoryPriority", static_cast<bool>(obj.memoryPriority));
}
void DumpVkPhysicalDeviceMeshShaderFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceMeshShaderFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(38);
    p.PrintKeyBool("taskShader", static_cast<bool>(obj.taskShader));
    p.PrintKeyBool("meshShader", static_cast<bool>(obj.meshShader));
    p.PrintKeyBool("multiviewMeshShader", static_cast<bool>(obj.multiviewMeshShader));
    p.PrintKeyBool("primitiveFragmentShadingRateMeshShader", static_cast<bool>(obj.primitiveFragmentShadingRateMeshShader));
    p.PrintKeyBool("meshShaderQueries", static_cast<bool>(obj.meshShaderQueries));
}
void DumpVkPhysicalDeviceMeshShaderPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceMeshShaderPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(37);
    p.PrintKeyValue("maxTaskWorkGroupTotalCount", obj.maxTaskWorkGroupTotalCount);
    {
        ArrayWrapper arr(p,"maxTaskWorkGroupCount", 3);
        for (uint32_t i = 0; i < 3; i++) { p.PrintElement(obj.maxTaskWorkGroupCount[i]); }
    }
    p.PrintKeyValue("maxTaskWorkGroupInvocations", obj.maxTaskWorkGroupInvocations);
    {
        ArrayWrapper arr(p,"maxTaskWorkGroupSize", 3);
        for (uint32_t i = 0; i < 3; i++) { p.PrintElement(obj.maxTaskWorkGroupSize[i]); }
    }
    p.PrintKeyValue("maxTaskPayloadSize", obj.maxTaskPayloadSize);
    p.PrintKeyValue("maxTaskSharedMemorySize", obj.maxTaskSharedMemorySize);
    p.PrintKeyValue("maxTaskPayloadAndSharedMemorySize", obj.maxTaskPayloadAndSharedMemorySize);
    p.PrintKeyValue("maxMeshWorkGroupTotalCount", obj.maxMeshWorkGroupTotalCount);
    {
        ArrayWrapper arr(p,"maxMeshWorkGroupCount", 3);
        for (uint32_t i = 0; i < 3; i++) { p.PrintElement(obj.maxMeshWorkGroupCount[i]); }
    }
    p.PrintKeyValue("maxMeshWorkGroupInvocations", obj.maxMeshWorkGroupInvocations);
    {
        ArrayWrapper arr(p,"maxMeshWorkGroupSize", 3);
        for (uint32_t i = 0; i < 3; i++) { p.PrintElement(obj.maxMeshWorkGroupSize[i]); }
    }
    p.PrintKeyValue("maxMeshSharedMemorySize", obj.maxMeshSharedMemorySize);
    p.PrintKeyValue("maxMeshPayloadAndSharedMemorySize", obj.maxMeshPayloadAndSharedMemorySize);
    p.PrintKeyValue("maxMeshOutputMemorySize", obj.maxMeshOutputMemorySize);
    p.PrintKeyValue("maxMeshPayloadAndOutputMemorySize", obj.maxMeshPayloadAndOutputMemorySize);
    p.PrintKeyValue("maxMeshOutputComponents", obj.maxMeshOutputComponents);
    p.PrintKeyValue("maxMeshOutputVertices", obj.maxMeshOutputVertices);
    p.PrintKeyValue("maxMeshOutputPrimitives", obj.maxMeshOutputPrimitives);
    p.PrintKeyValue("maxMeshOutputLayers", obj.maxMeshOutputLayers);
    p.PrintKeyValue("maxMeshMultiviewViewCount", obj.maxMeshMultiviewViewCount);
    p.PrintKeyValue("meshOutputPerVertexGranularity", obj.meshOutputPerVertexGranularity);
    p.PrintKeyValue("meshOutputPerPrimitiveGranularity", obj.meshOutputPerPrimitiveGranularity);
    p.PrintKeyValue("maxPreferredTaskWorkGroupInvocations", obj.maxPreferredTaskWorkGroupInvocations);
    p.PrintKeyValue("maxPreferredMeshWorkGroupInvocations", obj.maxPreferredMeshWorkGroupInvocations);
    p.PrintKeyBool("prefersLocalInvocationVertexOutput", static_cast<bool>(obj.prefersLocalInvocationVertexOutput));
    p.PrintKeyBool("prefersLocalInvocationPrimitiveOutput", static_cast<bool>(obj.prefersLocalInvocationPrimitiveOutput));
    p.PrintKeyBool("prefersCompactVertexOutput", static_cast<bool>(obj.prefersCompactVertexOutput));
    p.PrintKeyBool("prefersCompactPrimitiveOutput", static_cast<bool>(obj.prefersCompactPrimitiveOutput));
}
void DumpVkPhysicalDeviceMultiDrawFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceMultiDrawFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(9);
    p.PrintKeyBool("multiDraw", static_cast<bool>(obj.multiDraw));
}
void DumpVkPhysicalDeviceMultiDrawPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceMultiDrawPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyValue("maxMultiDrawCount", obj.maxMultiDrawCount);
}
void DumpVkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(33);
    p.PrintKeyBool("multisampledRenderToSingleSampled", static_cast<bool>(obj.multisampledRenderToSingleSampled));
}
void DumpVkPhysicalDeviceMultiviewFeatures(Printer &p, std::string name, const VkPhysicalDeviceMultiviewFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(27);
    p.PrintKeyBool("multiview", static_cast<bool>(obj.multiview));
    p.PrintKeyBool("multiviewGeometryShader", static_cast<bool>(obj.multiviewGeometryShader));
    p.PrintKeyBool("multiviewTessellationShader", static_cast<bool>(obj.multiviewTessellationShader));
}
void DumpVkPhysicalDeviceMultiviewProperties(Printer &p, std::string name, const VkPhysicalDeviceMultiviewProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(25);
    p.PrintKeyValue("maxMultiviewViewCount", obj.maxMultiviewViewCount);
    p.PrintKeyValue("maxMultiviewInstanceIndex", obj.maxMultiviewInstanceIndex);
}
void DumpVkPhysicalDeviceMutableDescriptorTypeFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(21);
    p.PrintKeyBool("mutableDescriptorType", static_cast<bool>(obj.mutableDescriptorType));
}
void DumpVkPhysicalDeviceNestedCommandBufferFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceNestedCommandBufferFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(34);
    p.PrintKeyBool("nestedCommandBuffer", static_cast<bool>(obj.nestedCommandBuffer));
    p.PrintKeyBool("nestedCommandBufferRendering", static_cast<bool>(obj.nestedCommandBufferRendering));
    p.PrintKeyBool("nestedCommandBufferSimultaneousUse", static_cast<bool>(obj.nestedCommandBufferSimultaneousUse));
}
void DumpVkPhysicalDeviceNestedCommandBufferPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceNestedCommandBufferPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyValue("maxCommandBufferNestingLevel", obj.maxCommandBufferNestingLevel);
}
void DumpVkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(18);
    p.PrintKeyBool("nonSeamlessCubeMap", static_cast<bool>(obj.nonSeamlessCubeMap));
}
void DumpVkPhysicalDeviceOpacityMicromapFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceOpacityMicromapFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(21);
    p.PrintKeyBool("micromap", static_cast<bool>(obj.micromap));
    p.PrintKeyBool("micromapCaptureReplay", static_cast<bool>(obj.micromapCaptureReplay));
    p.PrintKeyBool("micromapHostCommands", static_cast<bool>(obj.micromapHostCommands));
}
void DumpVkPhysicalDeviceOpacityMicromapPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceOpacityMicromapPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    p.PrintKeyValue("maxOpacity2StateSubdivisionLevel", obj.maxOpacity2StateSubdivisionLevel);
    p.PrintKeyValue("maxOpacity4StateSubdivisionLevel", obj.maxOpacity4StateSubdivisionLevel);
}
void DumpVkPhysicalDevicePCIBusInfoPropertiesEXT(Printer &p, std::string name, const VkPhysicalDevicePCIBusInfoPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(11);
    p.PrintKeyValue("pciDomain", obj.pciDomain);
    p.PrintKeyValue("pciBus", obj.pciBus);
    p.PrintKeyValue("pciDevice", obj.pciDevice);
    p.PrintKeyValue("pciFunction", obj.pciFunction);
}
void DumpVkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(Printer &p, std::string name, const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(25);
    p.PrintKeyBool("pageableDeviceLocalMemory", static_cast<bool>(obj.pageableDeviceLocalMemory));
}
void DumpVkPhysicalDevicePerformanceQueryFeaturesKHR(Printer &p, std::string name, const VkPhysicalDevicePerformanceQueryFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(36);
    p.PrintKeyBool("performanceCounterQueryPools", static_cast<bool>(obj.performanceCounterQueryPools));
    p.PrintKeyBool("performanceCounterMultipleQueryPools", static_cast<bool>(obj.performanceCounterMultipleQueryPools));
}
void DumpVkPhysicalDevicePerformanceQueryPropertiesKHR(Printer &p, std::string name, const VkPhysicalDevicePerformanceQueryPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(29);
    p.PrintKeyBool("allowCommandBufferQueryCopies", static_cast<bool>(obj.allowCommandBufferQueryCopies));
}
void DumpVkPhysicalDevicePipelineBinaryFeaturesKHR(Printer &p, std::string name, const VkPhysicalDevicePipelineBinaryFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    p.PrintKeyBool("pipelineBinaries", static_cast<bool>(obj.pipelineBinaries));
}
void DumpVkPhysicalDevicePipelineBinaryPropertiesKHR(Printer &p, std::string name, const VkPhysicalDevicePipelineBinaryPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(38);
    p.PrintKeyBool("pipelineBinaryInternalCache", static_cast<bool>(obj.pipelineBinaryInternalCache));
    p.PrintKeyBool("pipelineBinaryInternalCacheControl", static_cast<bool>(obj.pipelineBinaryInternalCacheControl));
    p.PrintKeyBool("pipelineBinaryPrefersInternalCache", static_cast<bool>(obj.pipelineBinaryPrefersInternalCache));
    p.PrintKeyBool("pipelineBinaryPrecompiledInternalCache", static_cast<bool>(obj.pipelineBinaryPrecompiledInternalCache));
    p.PrintKeyBool("pipelineBinaryCompressedData", static_cast<bool>(obj.pipelineBinaryCompressedData));
}
void DumpVkPhysicalDevicePipelineCreationCacheControlFeatures(Printer &p, std::string name, const VkPhysicalDevicePipelineCreationCacheControlFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyBool("pipelineCreationCacheControl", static_cast<bool>(obj.pipelineCreationCacheControl));
}
void DumpVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(Printer &p, std::string name, const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(22);
    p.PrintKeyBool("pipelineExecutableInfo", static_cast<bool>(obj.pipelineExecutableInfo));
}
void DumpVkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(Printer &p, std::string name, const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(27);
    p.PrintKeyBool("pipelineLibraryGroupHandles", static_cast<bool>(obj.pipelineLibraryGroupHandles));
}
void DumpVkPhysicalDevicePipelinePropertiesFeaturesEXT(Printer &p, std::string name, const VkPhysicalDevicePipelinePropertiesFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyBool("pipelinePropertiesIdentifier", static_cast<bool>(obj.pipelinePropertiesIdentifier));
}
void DumpVkPhysicalDevicePipelineProtectedAccessFeatures(Printer &p, std::string name, const VkPhysicalDevicePipelineProtectedAccessFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyBool("pipelineProtectedAccess", static_cast<bool>(obj.pipelineProtectedAccess));
}
void DumpVkPhysicalDevicePipelineRobustnessFeatures(Printer &p, std::string name, const VkPhysicalDevicePipelineRobustnessFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(18);
    p.PrintKeyBool("pipelineRobustness", static_cast<bool>(obj.pipelineRobustness));
}
void DumpVkPhysicalDevicePipelineRobustnessProperties(Printer &p, std::string name, const VkPhysicalDevicePipelineRobustnessProperties &obj) {
    ObjectWrapper object{p, name};
    DumpVkPipelineRobustnessBufferBehavior(p, "defaultRobustnessStorageBuffers", obj.defaultRobustnessStorageBuffers);
    DumpVkPipelineRobustnessBufferBehavior(p, "defaultRobustnessUniformBuffers", obj.defaultRobustnessUniformBuffers);
    DumpVkPipelineRobustnessBufferBehavior(p, "defaultRobustnessVertexInputs", obj.defaultRobustnessVertexInputs);
    DumpVkPipelineRobustnessImageBehavior(p, "defaultRobustnessImages", obj.defaultRobustnessImages);
}
void DumpVkPhysicalDevicePointClippingProperties(Printer &p, std::string name, const VkPhysicalDevicePointClippingProperties &obj) {
    ObjectWrapper object{p, name};
    DumpVkPointClippingBehavior(p, "pointClippingBehavior", obj.pointClippingBehavior);
}
#ifdef VK_ENABLE_BETA_EXTENSIONS
void DumpVkPhysicalDevicePortabilitySubsetFeaturesKHR(Printer &p, std::string name, const VkPhysicalDevicePortabilitySubsetFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(38);
    p.PrintKeyBool("constantAlphaColorBlendFactors", static_cast<bool>(obj.constantAlphaColorBlendFactors));
    p.PrintKeyBool("events", static_cast<bool>(obj.events));
    p.PrintKeyBool("imageViewFormatReinterpretation", static_cast<bool>(obj.imageViewFormatReinterpretation));
    p.PrintKeyBool("imageViewFormatSwizzle", static_cast<bool>(obj.imageViewFormatSwizzle));
    p.PrintKeyBool("imageView2DOn3DImage", static_cast<bool>(obj.imageView2DOn3DImage));
    p.PrintKeyBool("multisampleArrayImage", static_cast<bool>(obj.multisampleArrayImage));
    p.PrintKeyBool("mutableComparisonSamplers", static_cast<bool>(obj.mutableComparisonSamplers));
    p.PrintKeyBool("pointPolygons", static_cast<bool>(obj.pointPolygons));
    p.PrintKeyBool("samplerMipLodBias", static_cast<bool>(obj.samplerMipLodBias));
    p.PrintKeyBool("separateStencilMaskRef", static_cast<bool>(obj.separateStencilMaskRef));
    p.PrintKeyBool("shaderSampleRateInterpolationFunctions", static_cast<bool>(obj.shaderSampleRateInterpolationFunctions));
    p.PrintKeyBool("tessellationIsolines", static_cast<bool>(obj.tessellationIsolines));
    p.PrintKeyBool("tessellationPointMode", static_cast<bool>(obj.tessellationPointMode));
    p.PrintKeyBool("triangleFans", static_cast<bool>(obj.triangleFans));
    p.PrintKeyBool("vertexAttributeAccessBeyondStride", static_cast<bool>(obj.vertexAttributeAccessBeyondStride));
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void DumpVkPhysicalDevicePortabilitySubsetPropertiesKHR(Printer &p, std::string name, const VkPhysicalDevicePortabilitySubsetPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(36);
    p.PrintKeyValue("minVertexInputBindingStrideAlignment", obj.minVertexInputBindingStrideAlignment);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
void DumpVkPhysicalDevicePresentIdFeaturesKHR(Printer &p, std::string name, const VkPhysicalDevicePresentIdFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(9);
    p.PrintKeyBool("presentId", static_cast<bool>(obj.presentId));
}
void DumpVkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT(Printer &p, std::string name, const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(26);
    p.PrintKeyBool("presentModeFifoLatestReady", static_cast<bool>(obj.presentModeFifoLatestReady));
}
void DumpVkPhysicalDevicePresentWaitFeaturesKHR(Printer &p, std::string name, const VkPhysicalDevicePresentWaitFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(11);
    p.PrintKeyBool("presentWait", static_cast<bool>(obj.presentWait));
}
void DumpVkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(Printer &p, std::string name, const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(33);
    p.PrintKeyBool("primitiveTopologyListRestart", static_cast<bool>(obj.primitiveTopologyListRestart));
    p.PrintKeyBool("primitiveTopologyPatchListRestart", static_cast<bool>(obj.primitiveTopologyPatchListRestart));
}
void DumpVkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(Printer &p, std::string name, const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(45);
    p.PrintKeyBool("primitivesGeneratedQuery", static_cast<bool>(obj.primitivesGeneratedQuery));
    p.PrintKeyBool("primitivesGeneratedQueryWithRasterizerDiscard", static_cast<bool>(obj.primitivesGeneratedQueryWithRasterizerDiscard));
    p.PrintKeyBool("primitivesGeneratedQueryWithNonZeroStreams", static_cast<bool>(obj.primitivesGeneratedQueryWithNonZeroStreams));
}
void DumpVkPhysicalDevicePrivateDataFeatures(Printer &p, std::string name, const VkPhysicalDevicePrivateDataFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(11);
    p.PrintKeyBool("privateData", static_cast<bool>(obj.privateData));
}
void DumpVkPhysicalDeviceProtectedMemoryFeatures(Printer &p, std::string name, const VkPhysicalDeviceProtectedMemoryFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(15);
    p.PrintKeyBool("protectedMemory", static_cast<bool>(obj.protectedMemory));
}
void DumpVkPhysicalDeviceProtectedMemoryProperties(Printer &p, std::string name, const VkPhysicalDeviceProtectedMemoryProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    p.PrintKeyBool("protectedNoFault", static_cast<bool>(obj.protectedNoFault));
}
void DumpVkPhysicalDeviceProvokingVertexFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceProvokingVertexFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(41);
    p.PrintKeyBool("provokingVertexLast", static_cast<bool>(obj.provokingVertexLast));
    p.PrintKeyBool("transformFeedbackPreservesProvokingVertex", static_cast<bool>(obj.transformFeedbackPreservesProvokingVertex));
}
void DumpVkPhysicalDeviceProvokingVertexPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceProvokingVertexPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(52);
    p.PrintKeyBool("provokingVertexModePerPipeline", static_cast<bool>(obj.provokingVertexModePerPipeline));
    p.PrintKeyBool("transformFeedbackPreservesTriangleFanProvokingVertex", static_cast<bool>(obj.transformFeedbackPreservesTriangleFanProvokingVertex));
}
void DumpVkPhysicalDevicePushDescriptorProperties(Printer &p, std::string name, const VkPhysicalDevicePushDescriptorProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(18);
    p.PrintKeyValue("maxPushDescriptors", obj.maxPushDescriptors);
}
void DumpVkPhysicalDeviceRGBA10X6FormatsFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(33);
    p.PrintKeyBool("formatRgba10x6WithoutYCbCrSampler", static_cast<bool>(obj.formatRgba10x6WithoutYCbCrSampler));
}
void DumpVkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(41);
    p.PrintKeyBool("rasterizationOrderColorAttachmentAccess", static_cast<bool>(obj.rasterizationOrderColorAttachmentAccess));
    p.PrintKeyBool("rasterizationOrderDepthAttachmentAccess", static_cast<bool>(obj.rasterizationOrderDepthAttachmentAccess));
    p.PrintKeyBool("rasterizationOrderStencilAttachmentAccess", static_cast<bool>(obj.rasterizationOrderStencilAttachmentAccess));
}
void DumpVkPhysicalDeviceRayQueryFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceRayQueryFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(8);
    p.PrintKeyBool("rayQuery", static_cast<bool>(obj.rayQuery));
}
void DumpVkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(36);
    p.PrintKeyBool("rayTracingMaintenance1", static_cast<bool>(obj.rayTracingMaintenance1));
    p.PrintKeyBool("rayTracingPipelineTraceRaysIndirect2", static_cast<bool>(obj.rayTracingPipelineTraceRaysIndirect2));
}
void DumpVkPhysicalDeviceRayTracingPipelineFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceRayTracingPipelineFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(53);
    p.PrintKeyBool("rayTracingPipeline", static_cast<bool>(obj.rayTracingPipeline));
    p.PrintKeyBool("rayTracingPipelineShaderGroupHandleCaptureReplay", static_cast<bool>(obj.rayTracingPipelineShaderGroupHandleCaptureReplay));
    p.PrintKeyBool("rayTracingPipelineShaderGroupHandleCaptureReplayMixed", static_cast<bool>(obj.rayTracingPipelineShaderGroupHandleCaptureReplayMixed));
    p.PrintKeyBool("rayTracingPipelineTraceRaysIndirect", static_cast<bool>(obj.rayTracingPipelineTraceRaysIndirect));
    p.PrintKeyBool("rayTraversalPrimitiveCulling", static_cast<bool>(obj.rayTraversalPrimitiveCulling));
}
void DumpVkPhysicalDeviceRayTracingPipelinePropertiesKHR(Printer &p, std::string name, const VkPhysicalDeviceRayTracingPipelinePropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(34);
    p.PrintKeyValue("shaderGroupHandleSize", obj.shaderGroupHandleSize);
    p.PrintKeyValue("maxRayRecursionDepth", obj.maxRayRecursionDepth);
    p.PrintKeyValue("maxShaderGroupStride", obj.maxShaderGroupStride);
    p.PrintKeyValue("shaderGroupBaseAlignment", obj.shaderGroupBaseAlignment);
    p.PrintKeyValue("shaderGroupHandleCaptureReplaySize", obj.shaderGroupHandleCaptureReplaySize);
    p.PrintKeyValue("maxRayDispatchInvocationCount", obj.maxRayDispatchInvocationCount);
    p.PrintKeyValue("shaderGroupHandleAlignment", obj.shaderGroupHandleAlignment);
    p.PrintKeyValue("maxRayHitAttributeSize", obj.maxRayHitAttributeSize);
}
void DumpVkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyBool("rayTracingPositionFetch", static_cast<bool>(obj.rayTracingPositionFetch));
}
void DumpVkPhysicalDeviceRobustness2FeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceRobustness2FeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(19);
    p.PrintKeyBool("robustBufferAccess2", static_cast<bool>(obj.robustBufferAccess2));
    p.PrintKeyBool("robustImageAccess2", static_cast<bool>(obj.robustImageAccess2));
    p.PrintKeyBool("nullDescriptor", static_cast<bool>(obj.nullDescriptor));
}
void DumpVkPhysicalDeviceRobustness2PropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceRobustness2PropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(38);
    p.PrintKeyValue("robustStorageBufferAccessSizeAlignment", to_hex_str(p, obj.robustStorageBufferAccessSizeAlignment));
    p.PrintKeyValue("robustUniformBufferAccessSizeAlignment", to_hex_str(p, obj.robustUniformBufferAccessSizeAlignment));
}
void DumpVkPhysicalDeviceSampleLocationsPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceSampleLocationsPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    DumpVkSampleCountFlags(p, "sampleLocationSampleCounts", obj.sampleLocationSampleCounts);
    DumpVkExtent2D(p, "maxSampleLocationGridSize", obj.maxSampleLocationGridSize);
    {
        ArrayWrapper arr(p,"sampleLocationCoordinateRange", 2);
        for (uint32_t i = 0; i < 2; i++) { p.PrintElement(obj.sampleLocationCoordinateRange[i]); }
    }
    p.PrintKeyValue("sampleLocationSubPixelBits", obj.sampleLocationSubPixelBits);
    p.PrintKeyBool("variableSampleLocations", static_cast<bool>(obj.variableSampleLocations));
}
void DumpVkPhysicalDeviceSamplerFilterMinmaxProperties(Printer &p, std::string name, const VkPhysicalDeviceSamplerFilterMinmaxProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(34);
    p.PrintKeyBool("filterMinmaxSingleComponentFormats", static_cast<bool>(obj.filterMinmaxSingleComponentFormats));
    p.PrintKeyBool("filterMinmaxImageComponentMapping", static_cast<bool>(obj.filterMinmaxImageComponentMapping));
}
void DumpVkPhysicalDeviceSamplerYcbcrConversionFeatures(Printer &p, std::string name, const VkPhysicalDeviceSamplerYcbcrConversionFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(22);
    p.PrintKeyBool("samplerYcbcrConversion", static_cast<bool>(obj.samplerYcbcrConversion));
}
void DumpVkPhysicalDeviceScalarBlockLayoutFeatures(Printer &p, std::string name, const VkPhysicalDeviceScalarBlockLayoutFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("scalarBlockLayout", static_cast<bool>(obj.scalarBlockLayout));
}
void DumpVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(Printer &p, std::string name, const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(27);
    p.PrintKeyBool("separateDepthStencilLayouts", static_cast<bool>(obj.separateDepthStencilLayouts));
}
void DumpVkPhysicalDeviceShaderAtomicFloat2FeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(31);
    p.PrintKeyBool("shaderBufferFloat16Atomics", static_cast<bool>(obj.shaderBufferFloat16Atomics));
    p.PrintKeyBool("shaderBufferFloat16AtomicAdd", static_cast<bool>(obj.shaderBufferFloat16AtomicAdd));
    p.PrintKeyBool("shaderBufferFloat16AtomicMinMax", static_cast<bool>(obj.shaderBufferFloat16AtomicMinMax));
    p.PrintKeyBool("shaderBufferFloat32AtomicMinMax", static_cast<bool>(obj.shaderBufferFloat32AtomicMinMax));
    p.PrintKeyBool("shaderBufferFloat64AtomicMinMax", static_cast<bool>(obj.shaderBufferFloat64AtomicMinMax));
    p.PrintKeyBool("shaderSharedFloat16Atomics", static_cast<bool>(obj.shaderSharedFloat16Atomics));
    p.PrintKeyBool("shaderSharedFloat16AtomicAdd", static_cast<bool>(obj.shaderSharedFloat16AtomicAdd));
    p.PrintKeyBool("shaderSharedFloat16AtomicMinMax", static_cast<bool>(obj.shaderSharedFloat16AtomicMinMax));
    p.PrintKeyBool("shaderSharedFloat32AtomicMinMax", static_cast<bool>(obj.shaderSharedFloat32AtomicMinMax));
    p.PrintKeyBool("shaderSharedFloat64AtomicMinMax", static_cast<bool>(obj.shaderSharedFloat64AtomicMinMax));
    p.PrintKeyBool("shaderImageFloat32AtomicMinMax", static_cast<bool>(obj.shaderImageFloat32AtomicMinMax));
    p.PrintKeyBool("sparseImageFloat32AtomicMinMax", static_cast<bool>(obj.sparseImageFloat32AtomicMinMax));
}
void DumpVkPhysicalDeviceShaderAtomicFloatFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyBool("shaderBufferFloat32Atomics", static_cast<bool>(obj.shaderBufferFloat32Atomics));
    p.PrintKeyBool("shaderBufferFloat32AtomicAdd", static_cast<bool>(obj.shaderBufferFloat32AtomicAdd));
    p.PrintKeyBool("shaderBufferFloat64Atomics", static_cast<bool>(obj.shaderBufferFloat64Atomics));
    p.PrintKeyBool("shaderBufferFloat64AtomicAdd", static_cast<bool>(obj.shaderBufferFloat64AtomicAdd));
    p.PrintKeyBool("shaderSharedFloat32Atomics", static_cast<bool>(obj.shaderSharedFloat32Atomics));
    p.PrintKeyBool("shaderSharedFloat32AtomicAdd", static_cast<bool>(obj.shaderSharedFloat32AtomicAdd));
    p.PrintKeyBool("shaderSharedFloat64Atomics", static_cast<bool>(obj.shaderSharedFloat64Atomics));
    p.PrintKeyBool("shaderSharedFloat64AtomicAdd", static_cast<bool>(obj.shaderSharedFloat64AtomicAdd));
    p.PrintKeyBool("shaderImageFloat32Atomics", static_cast<bool>(obj.shaderImageFloat32Atomics));
    p.PrintKeyBool("shaderImageFloat32AtomicAdd", static_cast<bool>(obj.shaderImageFloat32AtomicAdd));
    p.PrintKeyBool("sparseImageFloat32Atomics", static_cast<bool>(obj.sparseImageFloat32Atomics));
    p.PrintKeyBool("sparseImageFloat32AtomicAdd", static_cast<bool>(obj.sparseImageFloat32AtomicAdd));
}
void DumpVkPhysicalDeviceShaderAtomicInt64Features(Printer &p, std::string name, const VkPhysicalDeviceShaderAtomicInt64Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(24);
    p.PrintKeyBool("shaderBufferInt64Atomics", static_cast<bool>(obj.shaderBufferInt64Atomics));
    p.PrintKeyBool("shaderSharedInt64Atomics", static_cast<bool>(obj.shaderSharedInt64Atomics));
}
void DumpVkPhysicalDeviceShaderClockFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceShaderClockFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(19);
    p.PrintKeyBool("shaderSubgroupClock", static_cast<bool>(obj.shaderSubgroupClock));
    p.PrintKeyBool("shaderDeviceClock", static_cast<bool>(obj.shaderDeviceClock));
}
void DumpVkPhysicalDeviceShaderDemoteToHelperInvocationFeatures(Printer &p, std::string name, const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(30);
    p.PrintKeyBool("shaderDemoteToHelperInvocation", static_cast<bool>(obj.shaderDemoteToHelperInvocation));
}
void DumpVkPhysicalDeviceShaderDrawParametersFeatures(Printer &p, std::string name, const VkPhysicalDeviceShaderDrawParametersFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("shaderDrawParameters", static_cast<bool>(obj.shaderDrawParameters));
}
void DumpVkPhysicalDeviceShaderExpectAssumeFeatures(Printer &p, std::string name, const VkPhysicalDeviceShaderExpectAssumeFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(18);
    p.PrintKeyBool("shaderExpectAssume", static_cast<bool>(obj.shaderExpectAssume));
}
void DumpVkPhysicalDeviceShaderFloat16Int8Features(Printer &p, std::string name, const VkPhysicalDeviceShaderFloat16Int8Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(13);
    p.PrintKeyBool("shaderFloat16", static_cast<bool>(obj.shaderFloat16));
    p.PrintKeyBool("shaderInt8", static_cast<bool>(obj.shaderInt8));
}
void DumpVkPhysicalDeviceShaderFloatControls2Features(Printer &p, std::string name, const VkPhysicalDeviceShaderFloatControls2Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("shaderFloatControls2", static_cast<bool>(obj.shaderFloatControls2));
}
void DumpVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyBool("shaderImageInt64Atomics", static_cast<bool>(obj.shaderImageInt64Atomics));
    p.PrintKeyBool("sparseImageInt64Atomics", static_cast<bool>(obj.sparseImageInt64Atomics));
}
void DumpVkPhysicalDeviceShaderIntegerDotProductFeatures(Printer &p, std::string name, const VkPhysicalDeviceShaderIntegerDotProductFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyBool("shaderIntegerDotProduct", static_cast<bool>(obj.shaderIntegerDotProduct));
}
void DumpVkPhysicalDeviceShaderIntegerDotProductProperties(Printer &p, std::string name, const VkPhysicalDeviceShaderIntegerDotProductProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(77);
    p.PrintKeyBool("integerDotProduct8BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct8BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct8BitSignedAccelerated", static_cast<bool>(obj.integerDotProduct8BitSignedAccelerated));
    p.PrintKeyBool("integerDotProduct8BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct8BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProduct4x8BitPackedUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct4x8BitPackedUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct4x8BitPackedSignedAccelerated", static_cast<bool>(obj.integerDotProduct4x8BitPackedSignedAccelerated));
    p.PrintKeyBool("integerDotProduct4x8BitPackedMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct4x8BitPackedMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProduct16BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct16BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct16BitSignedAccelerated", static_cast<bool>(obj.integerDotProduct16BitSignedAccelerated));
    p.PrintKeyBool("integerDotProduct16BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct16BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProduct32BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct32BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct32BitSignedAccelerated", static_cast<bool>(obj.integerDotProduct32BitSignedAccelerated));
    p.PrintKeyBool("integerDotProduct32BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct32BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProduct64BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct64BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct64BitSignedAccelerated", static_cast<bool>(obj.integerDotProduct64BitSignedAccelerated));
    p.PrintKeyBool("integerDotProduct64BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct64BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating8BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating8BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating8BitSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating8BitSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating8BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating8BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating4x8BitPackedUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating4x8BitPackedUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating4x8BitPackedSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating4x8BitPackedSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating4x8BitPackedMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating4x8BitPackedMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating16BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating16BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating16BitSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating16BitSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating16BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating16BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating32BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating32BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating32BitSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating32BitSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating32BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating32BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating64BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating64BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating64BitSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating64BitSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating64BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating64BitMixedSignednessAccelerated));
}
void DumpVkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(26);
    p.PrintKeyBool("shaderMaximalReconvergence", static_cast<bool>(obj.shaderMaximalReconvergence));
}
void DumpVkPhysicalDeviceShaderModuleIdentifierFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(22);
    p.PrintKeyBool("shaderModuleIdentifier", static_cast<bool>(obj.shaderModuleIdentifier));
}
void DumpVkPhysicalDeviceShaderModuleIdentifierPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(39);
    p.PrintKeyValue("shaderModuleIdentifierAlgorithmUUID", obj.shaderModuleIdentifierAlgorithmUUID);
}
void DumpVkPhysicalDeviceShaderObjectFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderObjectFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(12);
    p.PrintKeyBool("shaderObject", static_cast<bool>(obj.shaderObject));
}
void DumpVkPhysicalDeviceShaderObjectPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderObjectPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyValue("shaderBinaryUUID", obj.shaderBinaryUUID);
    p.PrintKeyValue("shaderBinaryVersion", obj.shaderBinaryVersion);
}
void DumpVkPhysicalDeviceShaderQuadControlFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceShaderQuadControlFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("shaderQuadControl", static_cast<bool>(obj.shaderQuadControl));
}
void DumpVkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    p.PrintKeyBool("shaderRelaxedExtendedInstruction", static_cast<bool>(obj.shaderRelaxedExtendedInstruction));
}
void DumpVkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(26);
    p.PrintKeyBool("shaderReplicatedComposites", static_cast<bool>(obj.shaderReplicatedComposites));
}
void DumpVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(Printer &p, std::string name, const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(27);
    p.PrintKeyBool("shaderSubgroupExtendedTypes", static_cast<bool>(obj.shaderSubgroupExtendedTypes));
}
void DumpVkPhysicalDeviceShaderSubgroupRotateFeatures(Printer &p, std::string name, const VkPhysicalDeviceShaderSubgroupRotateFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(29);
    p.PrintKeyBool("shaderSubgroupRotate", static_cast<bool>(obj.shaderSubgroupRotate));
    p.PrintKeyBool("shaderSubgroupRotateClustered", static_cast<bool>(obj.shaderSubgroupRotateClustered));
}
void DumpVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    p.PrintKeyBool("shaderSubgroupUniformControlFlow", static_cast<bool>(obj.shaderSubgroupUniformControlFlow));
}
void DumpVkPhysicalDeviceShaderTerminateInvocationFeatures(Printer &p, std::string name, const VkPhysicalDeviceShaderTerminateInvocationFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(25);
    p.PrintKeyBool("shaderTerminateInvocation", static_cast<bool>(obj.shaderTerminateInvocation));
}
void DumpVkPhysicalDeviceShaderTileImageFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderTileImageFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    p.PrintKeyBool("shaderTileImageColorReadAccess", static_cast<bool>(obj.shaderTileImageColorReadAccess));
    p.PrintKeyBool("shaderTileImageDepthReadAccess", static_cast<bool>(obj.shaderTileImageDepthReadAccess));
    p.PrintKeyBool("shaderTileImageStencilReadAccess", static_cast<bool>(obj.shaderTileImageStencilReadAccess));
}
void DumpVkPhysicalDeviceShaderTileImagePropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceShaderTileImagePropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(48);
    p.PrintKeyBool("shaderTileImageCoherentReadAccelerated", static_cast<bool>(obj.shaderTileImageCoherentReadAccelerated));
    p.PrintKeyBool("shaderTileImageReadSampleFromPixelRateInvocation", static_cast<bool>(obj.shaderTileImageReadSampleFromPixelRateInvocation));
    p.PrintKeyBool("shaderTileImageReadFromHelperInvocation", static_cast<bool>(obj.shaderTileImageReadFromHelperInvocation));
}
void DumpVkPhysicalDeviceSparseProperties(Printer &p, std::string name, const VkPhysicalDeviceSparseProperties &obj) {
    if (p.Type() == OutputType::json)
        p.ObjectStart("sparseProperties");
    else
        p.SetSubHeader().ObjectStart(name);
    p.SetMinKeyWidth(40);
    p.PrintKeyBool("residencyStandard2DBlockShape", static_cast<bool>(obj.residencyStandard2DBlockShape));
    p.PrintKeyBool("residencyStandard2DMultisampleBlockShape", static_cast<bool>(obj.residencyStandard2DMultisampleBlockShape));
    p.PrintKeyBool("residencyStandard3DBlockShape", static_cast<bool>(obj.residencyStandard3DBlockShape));
    p.PrintKeyBool("residencyAlignedMipSize", static_cast<bool>(obj.residencyAlignedMipSize));
    p.PrintKeyBool("residencyNonResidentStrict", static_cast<bool>(obj.residencyNonResidentStrict));
    p.ObjectEnd();
}
void DumpVkPhysicalDeviceSubgroupProperties(Printer &p, std::string name, const VkPhysicalDeviceSubgroupProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(25);
    p.PrintKeyValue("subgroupSize", obj.subgroupSize);
    DumpVkShaderStageFlags(p, "supportedStages", obj.supportedStages);
    DumpVkSubgroupFeatureFlags(p, "supportedOperations", obj.supportedOperations);
    p.PrintKeyBool("quadOperationsInAllStages", static_cast<bool>(obj.quadOperationsInAllStages));
}
void DumpVkPhysicalDeviceSubgroupSizeControlFeatures(Printer &p, std::string name, const VkPhysicalDeviceSubgroupSizeControlFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("subgroupSizeControl", static_cast<bool>(obj.subgroupSizeControl));
    p.PrintKeyBool("computeFullSubgroups", static_cast<bool>(obj.computeFullSubgroups));
}
void DumpVkPhysicalDeviceSubgroupSizeControlProperties(Printer &p, std::string name, const VkPhysicalDeviceSubgroupSizeControlProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyValue("minSubgroupSize", obj.minSubgroupSize);
    p.PrintKeyValue("maxSubgroupSize", obj.maxSubgroupSize);
    p.PrintKeyValue("maxComputeWorkgroupSubgroups", obj.maxComputeWorkgroupSubgroups);
    DumpVkShaderStageFlags(p, "requiredSubgroupSizeStages", obj.requiredSubgroupSizeStages);
}
void DumpVkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("subpassMergeFeedback", static_cast<bool>(obj.subpassMergeFeedback));
}
void DumpVkPhysicalDeviceSwapchainMaintenance1FeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(21);
    p.PrintKeyBool("swapchainMaintenance1", static_cast<bool>(obj.swapchainMaintenance1));
}
void DumpVkPhysicalDeviceSynchronization2Features(Printer &p, std::string name, const VkPhysicalDeviceSynchronization2Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    p.PrintKeyBool("synchronization2", static_cast<bool>(obj.synchronization2));
}
void DumpVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    p.PrintKeyBool("texelBufferAlignment", static_cast<bool>(obj.texelBufferAlignment));
}
void DumpVkPhysicalDeviceTexelBufferAlignmentProperties(Printer &p, std::string name, const VkPhysicalDeviceTexelBufferAlignmentProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(44);
    p.PrintKeyValue("storageTexelBufferOffsetAlignmentBytes", to_hex_str(p, obj.storageTexelBufferOffsetAlignmentBytes));
    p.PrintKeyBool("storageTexelBufferOffsetSingleTexelAlignment", static_cast<bool>(obj.storageTexelBufferOffsetSingleTexelAlignment));
    p.PrintKeyValue("uniformTexelBufferOffsetAlignmentBytes", to_hex_str(p, obj.uniformTexelBufferOffsetAlignmentBytes));
    p.PrintKeyBool("uniformTexelBufferOffsetSingleTexelAlignment", static_cast<bool>(obj.uniformTexelBufferOffsetSingleTexelAlignment));
}
void DumpVkPhysicalDeviceTextureCompressionASTCHDRFeatures(Printer &p, std::string name, const VkPhysicalDeviceTextureCompressionASTCHDRFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(26);
    p.PrintKeyBool("textureCompressionASTC_HDR", static_cast<bool>(obj.textureCompressionASTC_HDR));
}
void DumpVkPhysicalDeviceTimelineSemaphoreFeatures(Printer &p, std::string name, const VkPhysicalDeviceTimelineSemaphoreFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("timelineSemaphore", static_cast<bool>(obj.timelineSemaphore));
}
void DumpVkPhysicalDeviceTimelineSemaphoreProperties(Printer &p, std::string name, const VkPhysicalDeviceTimelineSemaphoreProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(35);
    p.PrintKeyValue("maxTimelineSemaphoreValueDifference", obj.maxTimelineSemaphoreValueDifference);
}
void DumpVkPhysicalDeviceToolProperties(Printer &p, std::string name, const VkPhysicalDeviceToolProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    p.PrintKeyString("name", obj.name);
    p.PrintKeyString("version", obj.version);
    DumpVkToolPurposeFlags(p, "purposes", obj.purposes);
    p.PrintKeyString("description", obj.description);
    p.PrintKeyString("layer", obj.layer);
}
void DumpVkPhysicalDeviceTransformFeedbackFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceTransformFeedbackFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("transformFeedback", static_cast<bool>(obj.transformFeedback));
    p.PrintKeyBool("geometryStreams", static_cast<bool>(obj.geometryStreams));
}
void DumpVkPhysicalDeviceTransformFeedbackPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceTransformFeedbackPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(42);
    p.PrintKeyValue("maxTransformFeedbackStreams", obj.maxTransformFeedbackStreams);
    p.PrintKeyValue("maxTransformFeedbackBuffers", obj.maxTransformFeedbackBuffers);
    p.PrintKeyValue("maxTransformFeedbackBufferSize", to_hex_str(p, obj.maxTransformFeedbackBufferSize));
    p.PrintKeyValue("maxTransformFeedbackStreamDataSize", obj.maxTransformFeedbackStreamDataSize);
    p.PrintKeyValue("maxTransformFeedbackBufferDataSize", obj.maxTransformFeedbackBufferDataSize);
    p.PrintKeyValue("maxTransformFeedbackBufferDataStride", obj.maxTransformFeedbackBufferDataStride);
    p.PrintKeyBool("transformFeedbackQueries", static_cast<bool>(obj.transformFeedbackQueries));
    p.PrintKeyBool("transformFeedbackStreamsLinesTriangles", static_cast<bool>(obj.transformFeedbackStreamsLinesTriangles));
    p.PrintKeyBool("transformFeedbackRasterizationStreamSelect", static_cast<bool>(obj.transformFeedbackRasterizationStreamSelect));
    p.PrintKeyBool("transformFeedbackDraw", static_cast<bool>(obj.transformFeedbackDraw));
}
void DumpVkPhysicalDeviceUniformBufferStandardLayoutFeatures(Printer &p, std::string name, const VkPhysicalDeviceUniformBufferStandardLayoutFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(27);
    p.PrintKeyBool("uniformBufferStandardLayout", static_cast<bool>(obj.uniformBufferStandardLayout));
}
void DumpVkPhysicalDeviceVariablePointersFeatures(Printer &p, std::string name, const VkPhysicalDeviceVariablePointersFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(29);
    p.PrintKeyBool("variablePointersStorageBuffer", static_cast<bool>(obj.variablePointersStorageBuffer));
    p.PrintKeyBool("variablePointers", static_cast<bool>(obj.variablePointers));
}
void DumpVkPhysicalDeviceVertexAttributeDivisorFeatures(Printer &p, std::string name, const VkPhysicalDeviceVertexAttributeDivisorFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(38);
    p.PrintKeyBool("vertexAttributeInstanceRateDivisor", static_cast<bool>(obj.vertexAttributeInstanceRateDivisor));
    p.PrintKeyBool("vertexAttributeInstanceRateZeroDivisor", static_cast<bool>(obj.vertexAttributeInstanceRateZeroDivisor));
}
void DumpVkPhysicalDeviceVertexAttributeDivisorProperties(Printer &p, std::string name, const VkPhysicalDeviceVertexAttributeDivisorProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyValue("maxVertexAttribDivisor", obj.maxVertexAttribDivisor);
    p.PrintKeyBool("supportsNonZeroFirstInstance", static_cast<bool>(obj.supportsNonZeroFirstInstance));
}
void DumpVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(Printer &p, std::string name, const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(22);
    p.PrintKeyValue("maxVertexAttribDivisor", obj.maxVertexAttribDivisor);
}
void DumpVkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(25);
    p.PrintKeyBool("vertexAttributeRobustness", static_cast<bool>(obj.vertexAttributeRobustness));
}
void DumpVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(23);
    p.PrintKeyBool("vertexInputDynamicState", static_cast<bool>(obj.vertexInputDynamicState));
}
void DumpVkPhysicalDeviceVideoEncodeAV1FeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(14);
    p.PrintKeyBool("videoEncodeAV1", static_cast<bool>(obj.videoEncodeAV1));
}
void DumpVkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(26);
    p.PrintKeyBool("videoEncodeQuantizationMap", static_cast<bool>(obj.videoEncodeQuantizationMap));
}
void DumpVkPhysicalDeviceVideoMaintenance1FeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceVideoMaintenance1FeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("videoMaintenance1", static_cast<bool>(obj.videoMaintenance1));
}
void DumpVkPhysicalDeviceVideoMaintenance2FeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceVideoMaintenance2FeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("videoMaintenance2", static_cast<bool>(obj.videoMaintenance2));
}
void DumpVkPhysicalDeviceVulkan11Features(Printer &p, std::string name, const VkPhysicalDeviceVulkan11Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(34);
    p.PrintKeyBool("storageBuffer16BitAccess", static_cast<bool>(obj.storageBuffer16BitAccess));
    p.PrintKeyBool("uniformAndStorageBuffer16BitAccess", static_cast<bool>(obj.uniformAndStorageBuffer16BitAccess));
    p.PrintKeyBool("storagePushConstant16", static_cast<bool>(obj.storagePushConstant16));
    p.PrintKeyBool("storageInputOutput16", static_cast<bool>(obj.storageInputOutput16));
    p.PrintKeyBool("multiview", static_cast<bool>(obj.multiview));
    p.PrintKeyBool("multiviewGeometryShader", static_cast<bool>(obj.multiviewGeometryShader));
    p.PrintKeyBool("multiviewTessellationShader", static_cast<bool>(obj.multiviewTessellationShader));
    p.PrintKeyBool("variablePointersStorageBuffer", static_cast<bool>(obj.variablePointersStorageBuffer));
    p.PrintKeyBool("variablePointers", static_cast<bool>(obj.variablePointers));
    p.PrintKeyBool("protectedMemory", static_cast<bool>(obj.protectedMemory));
    p.PrintKeyBool("samplerYcbcrConversion", static_cast<bool>(obj.samplerYcbcrConversion));
    p.PrintKeyBool("shaderDrawParameters", static_cast<bool>(obj.shaderDrawParameters));
}
void DumpVkPhysicalDeviceVulkan11Properties(Printer &p, std::string name, const VkPhysicalDeviceVulkan11Properties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(33);
    p.PrintKeyValue("deviceUUID", obj.deviceUUID);
    p.PrintKeyValue("driverUUID", obj.driverUUID);
    if (obj.deviceLUIDValid) { // special case
    p.PrintKeyValue("deviceLUID", obj.deviceLUID);
    }
    p.PrintKeyValue("deviceNodeMask", obj.deviceNodeMask);
    p.PrintKeyBool("deviceLUIDValid", static_cast<bool>(obj.deviceLUIDValid));
    p.PrintKeyValue("subgroupSize", obj.subgroupSize);
    DumpVkShaderStageFlags(p, "subgroupSupportedStages", obj.subgroupSupportedStages);
    DumpVkSubgroupFeatureFlags(p, "subgroupSupportedOperations", obj.subgroupSupportedOperations);
    p.PrintKeyBool("subgroupQuadOperationsInAllStages", static_cast<bool>(obj.subgroupQuadOperationsInAllStages));
    DumpVkPointClippingBehavior(p, "pointClippingBehavior", obj.pointClippingBehavior);
    p.PrintKeyValue("maxMultiviewViewCount", obj.maxMultiviewViewCount);
    p.PrintKeyValue("maxMultiviewInstanceIndex", obj.maxMultiviewInstanceIndex);
    p.PrintKeyBool("protectedNoFault", static_cast<bool>(obj.protectedNoFault));
    p.PrintKeyValue("maxPerSetDescriptors", obj.maxPerSetDescriptors);
    p.PrintKeyValue("maxMemoryAllocationSize", to_hex_str(p, obj.maxMemoryAllocationSize));
}
void DumpVkPhysicalDeviceVulkan12Features(Printer &p, std::string name, const VkPhysicalDeviceVulkan12Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(50);
    p.PrintKeyBool("samplerMirrorClampToEdge", static_cast<bool>(obj.samplerMirrorClampToEdge));
    p.PrintKeyBool("drawIndirectCount", static_cast<bool>(obj.drawIndirectCount));
    p.PrintKeyBool("storageBuffer8BitAccess", static_cast<bool>(obj.storageBuffer8BitAccess));
    p.PrintKeyBool("uniformAndStorageBuffer8BitAccess", static_cast<bool>(obj.uniformAndStorageBuffer8BitAccess));
    p.PrintKeyBool("storagePushConstant8", static_cast<bool>(obj.storagePushConstant8));
    p.PrintKeyBool("shaderBufferInt64Atomics", static_cast<bool>(obj.shaderBufferInt64Atomics));
    p.PrintKeyBool("shaderSharedInt64Atomics", static_cast<bool>(obj.shaderSharedInt64Atomics));
    p.PrintKeyBool("shaderFloat16", static_cast<bool>(obj.shaderFloat16));
    p.PrintKeyBool("shaderInt8", static_cast<bool>(obj.shaderInt8));
    p.PrintKeyBool("descriptorIndexing", static_cast<bool>(obj.descriptorIndexing));
    p.PrintKeyBool("shaderInputAttachmentArrayDynamicIndexing", static_cast<bool>(obj.shaderInputAttachmentArrayDynamicIndexing));
    p.PrintKeyBool("shaderUniformTexelBufferArrayDynamicIndexing", static_cast<bool>(obj.shaderUniformTexelBufferArrayDynamicIndexing));
    p.PrintKeyBool("shaderStorageTexelBufferArrayDynamicIndexing", static_cast<bool>(obj.shaderStorageTexelBufferArrayDynamicIndexing));
    p.PrintKeyBool("shaderUniformBufferArrayNonUniformIndexing", static_cast<bool>(obj.shaderUniformBufferArrayNonUniformIndexing));
    p.PrintKeyBool("shaderSampledImageArrayNonUniformIndexing", static_cast<bool>(obj.shaderSampledImageArrayNonUniformIndexing));
    p.PrintKeyBool("shaderStorageBufferArrayNonUniformIndexing", static_cast<bool>(obj.shaderStorageBufferArrayNonUniformIndexing));
    p.PrintKeyBool("shaderStorageImageArrayNonUniformIndexing", static_cast<bool>(obj.shaderStorageImageArrayNonUniformIndexing));
    p.PrintKeyBool("shaderInputAttachmentArrayNonUniformIndexing", static_cast<bool>(obj.shaderInputAttachmentArrayNonUniformIndexing));
    p.PrintKeyBool("shaderUniformTexelBufferArrayNonUniformIndexing", static_cast<bool>(obj.shaderUniformTexelBufferArrayNonUniformIndexing));
    p.PrintKeyBool("shaderStorageTexelBufferArrayNonUniformIndexing", static_cast<bool>(obj.shaderStorageTexelBufferArrayNonUniformIndexing));
    p.PrintKeyBool("descriptorBindingUniformBufferUpdateAfterBind", static_cast<bool>(obj.descriptorBindingUniformBufferUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingSampledImageUpdateAfterBind", static_cast<bool>(obj.descriptorBindingSampledImageUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingStorageImageUpdateAfterBind", static_cast<bool>(obj.descriptorBindingStorageImageUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingStorageBufferUpdateAfterBind", static_cast<bool>(obj.descriptorBindingStorageBufferUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingUniformTexelBufferUpdateAfterBind", static_cast<bool>(obj.descriptorBindingUniformTexelBufferUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingStorageTexelBufferUpdateAfterBind", static_cast<bool>(obj.descriptorBindingStorageTexelBufferUpdateAfterBind));
    p.PrintKeyBool("descriptorBindingUpdateUnusedWhilePending", static_cast<bool>(obj.descriptorBindingUpdateUnusedWhilePending));
    p.PrintKeyBool("descriptorBindingPartiallyBound", static_cast<bool>(obj.descriptorBindingPartiallyBound));
    p.PrintKeyBool("descriptorBindingVariableDescriptorCount", static_cast<bool>(obj.descriptorBindingVariableDescriptorCount));
    p.PrintKeyBool("runtimeDescriptorArray", static_cast<bool>(obj.runtimeDescriptorArray));
    p.PrintKeyBool("samplerFilterMinmax", static_cast<bool>(obj.samplerFilterMinmax));
    p.PrintKeyBool("scalarBlockLayout", static_cast<bool>(obj.scalarBlockLayout));
    p.PrintKeyBool("imagelessFramebuffer", static_cast<bool>(obj.imagelessFramebuffer));
    p.PrintKeyBool("uniformBufferStandardLayout", static_cast<bool>(obj.uniformBufferStandardLayout));
    p.PrintKeyBool("shaderSubgroupExtendedTypes", static_cast<bool>(obj.shaderSubgroupExtendedTypes));
    p.PrintKeyBool("separateDepthStencilLayouts", static_cast<bool>(obj.separateDepthStencilLayouts));
    p.PrintKeyBool("hostQueryReset", static_cast<bool>(obj.hostQueryReset));
    p.PrintKeyBool("timelineSemaphore", static_cast<bool>(obj.timelineSemaphore));
    p.PrintKeyBool("bufferDeviceAddress", static_cast<bool>(obj.bufferDeviceAddress));
    p.PrintKeyBool("bufferDeviceAddressCaptureReplay", static_cast<bool>(obj.bufferDeviceAddressCaptureReplay));
    p.PrintKeyBool("bufferDeviceAddressMultiDevice", static_cast<bool>(obj.bufferDeviceAddressMultiDevice));
    p.PrintKeyBool("vulkanMemoryModel", static_cast<bool>(obj.vulkanMemoryModel));
    p.PrintKeyBool("vulkanMemoryModelDeviceScope", static_cast<bool>(obj.vulkanMemoryModelDeviceScope));
    p.PrintKeyBool("vulkanMemoryModelAvailabilityVisibilityChains", static_cast<bool>(obj.vulkanMemoryModelAvailabilityVisibilityChains));
    p.PrintKeyBool("shaderOutputViewportIndex", static_cast<bool>(obj.shaderOutputViewportIndex));
    p.PrintKeyBool("shaderOutputLayer", static_cast<bool>(obj.shaderOutputLayer));
    p.PrintKeyBool("subgroupBroadcastDynamicId", static_cast<bool>(obj.subgroupBroadcastDynamicId));
}
void DumpVkPhysicalDeviceVulkan12Properties(Printer &p, std::string name, const VkPhysicalDeviceVulkan12Properties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(52);
    DumpVkDriverId(p, "driverID", obj.driverID);
    p.PrintKeyString("driverName", obj.driverName);
    p.PrintKeyString("driverInfo", obj.driverInfo);
    DumpVkConformanceVersion(p, "conformanceVersion", obj.conformanceVersion);
    DumpVkShaderFloatControlsIndependence(p, "denormBehaviorIndependence", obj.denormBehaviorIndependence);
    DumpVkShaderFloatControlsIndependence(p, "roundingModeIndependence", obj.roundingModeIndependence);
    p.PrintKeyBool("shaderSignedZeroInfNanPreserveFloat16", static_cast<bool>(obj.shaderSignedZeroInfNanPreserveFloat16));
    p.PrintKeyBool("shaderSignedZeroInfNanPreserveFloat32", static_cast<bool>(obj.shaderSignedZeroInfNanPreserveFloat32));
    p.PrintKeyBool("shaderSignedZeroInfNanPreserveFloat64", static_cast<bool>(obj.shaderSignedZeroInfNanPreserveFloat64));
    p.PrintKeyBool("shaderDenormPreserveFloat16", static_cast<bool>(obj.shaderDenormPreserveFloat16));
    p.PrintKeyBool("shaderDenormPreserveFloat32", static_cast<bool>(obj.shaderDenormPreserveFloat32));
    p.PrintKeyBool("shaderDenormPreserveFloat64", static_cast<bool>(obj.shaderDenormPreserveFloat64));
    p.PrintKeyBool("shaderDenormFlushToZeroFloat16", static_cast<bool>(obj.shaderDenormFlushToZeroFloat16));
    p.PrintKeyBool("shaderDenormFlushToZeroFloat32", static_cast<bool>(obj.shaderDenormFlushToZeroFloat32));
    p.PrintKeyBool("shaderDenormFlushToZeroFloat64", static_cast<bool>(obj.shaderDenormFlushToZeroFloat64));
    p.PrintKeyBool("shaderRoundingModeRTEFloat16", static_cast<bool>(obj.shaderRoundingModeRTEFloat16));
    p.PrintKeyBool("shaderRoundingModeRTEFloat32", static_cast<bool>(obj.shaderRoundingModeRTEFloat32));
    p.PrintKeyBool("shaderRoundingModeRTEFloat64", static_cast<bool>(obj.shaderRoundingModeRTEFloat64));
    p.PrintKeyBool("shaderRoundingModeRTZFloat16", static_cast<bool>(obj.shaderRoundingModeRTZFloat16));
    p.PrintKeyBool("shaderRoundingModeRTZFloat32", static_cast<bool>(obj.shaderRoundingModeRTZFloat32));
    p.PrintKeyBool("shaderRoundingModeRTZFloat64", static_cast<bool>(obj.shaderRoundingModeRTZFloat64));
    p.PrintKeyValue("maxUpdateAfterBindDescriptorsInAllPools", obj.maxUpdateAfterBindDescriptorsInAllPools);
    p.PrintKeyBool("shaderUniformBufferArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderUniformBufferArrayNonUniformIndexingNative));
    p.PrintKeyBool("shaderSampledImageArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderSampledImageArrayNonUniformIndexingNative));
    p.PrintKeyBool("shaderStorageBufferArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderStorageBufferArrayNonUniformIndexingNative));
    p.PrintKeyBool("shaderStorageImageArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderStorageImageArrayNonUniformIndexingNative));
    p.PrintKeyBool("shaderInputAttachmentArrayNonUniformIndexingNative", static_cast<bool>(obj.shaderInputAttachmentArrayNonUniformIndexingNative));
    p.PrintKeyBool("robustBufferAccessUpdateAfterBind", static_cast<bool>(obj.robustBufferAccessUpdateAfterBind));
    p.PrintKeyBool("quadDivergentImplicitLod", static_cast<bool>(obj.quadDivergentImplicitLod));
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindSamplers", obj.maxPerStageDescriptorUpdateAfterBindSamplers);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindUniformBuffers", obj.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindStorageBuffers", obj.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindSampledImages", obj.maxPerStageDescriptorUpdateAfterBindSampledImages);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindStorageImages", obj.maxPerStageDescriptorUpdateAfterBindStorageImages);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindInputAttachments", obj.maxPerStageDescriptorUpdateAfterBindInputAttachments);
    p.PrintKeyValue("maxPerStageUpdateAfterBindResources", obj.maxPerStageUpdateAfterBindResources);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindSamplers", obj.maxDescriptorSetUpdateAfterBindSamplers);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindUniformBuffers", obj.maxDescriptorSetUpdateAfterBindUniformBuffers);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindUniformBuffersDynamic", obj.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindStorageBuffers", obj.maxDescriptorSetUpdateAfterBindStorageBuffers);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindStorageBuffersDynamic", obj.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindSampledImages", obj.maxDescriptorSetUpdateAfterBindSampledImages);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindStorageImages", obj.maxDescriptorSetUpdateAfterBindStorageImages);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindInputAttachments", obj.maxDescriptorSetUpdateAfterBindInputAttachments);
    DumpVkResolveModeFlags(p, "supportedDepthResolveModes", obj.supportedDepthResolveModes);
    DumpVkResolveModeFlags(p, "supportedStencilResolveModes", obj.supportedStencilResolveModes);
    p.PrintKeyBool("independentResolveNone", static_cast<bool>(obj.independentResolveNone));
    p.PrintKeyBool("independentResolve", static_cast<bool>(obj.independentResolve));
    p.PrintKeyBool("filterMinmaxSingleComponentFormats", static_cast<bool>(obj.filterMinmaxSingleComponentFormats));
    p.PrintKeyBool("filterMinmaxImageComponentMapping", static_cast<bool>(obj.filterMinmaxImageComponentMapping));
    p.PrintKeyValue("maxTimelineSemaphoreValueDifference", obj.maxTimelineSemaphoreValueDifference);
    DumpVkSampleCountFlags(p, "framebufferIntegerColorSampleCounts", obj.framebufferIntegerColorSampleCounts);
}
void DumpVkPhysicalDeviceVulkan13Features(Printer &p, std::string name, const VkPhysicalDeviceVulkan13Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(50);
    p.PrintKeyBool("robustImageAccess", static_cast<bool>(obj.robustImageAccess));
    p.PrintKeyBool("inlineUniformBlock", static_cast<bool>(obj.inlineUniformBlock));
    p.PrintKeyBool("descriptorBindingInlineUniformBlockUpdateAfterBind", static_cast<bool>(obj.descriptorBindingInlineUniformBlockUpdateAfterBind));
    p.PrintKeyBool("pipelineCreationCacheControl", static_cast<bool>(obj.pipelineCreationCacheControl));
    p.PrintKeyBool("privateData", static_cast<bool>(obj.privateData));
    p.PrintKeyBool("shaderDemoteToHelperInvocation", static_cast<bool>(obj.shaderDemoteToHelperInvocation));
    p.PrintKeyBool("shaderTerminateInvocation", static_cast<bool>(obj.shaderTerminateInvocation));
    p.PrintKeyBool("subgroupSizeControl", static_cast<bool>(obj.subgroupSizeControl));
    p.PrintKeyBool("computeFullSubgroups", static_cast<bool>(obj.computeFullSubgroups));
    p.PrintKeyBool("synchronization2", static_cast<bool>(obj.synchronization2));
    p.PrintKeyBool("textureCompressionASTC_HDR", static_cast<bool>(obj.textureCompressionASTC_HDR));
    p.PrintKeyBool("shaderZeroInitializeWorkgroupMemory", static_cast<bool>(obj.shaderZeroInitializeWorkgroupMemory));
    p.PrintKeyBool("dynamicRendering", static_cast<bool>(obj.dynamicRendering));
    p.PrintKeyBool("shaderIntegerDotProduct", static_cast<bool>(obj.shaderIntegerDotProduct));
    p.PrintKeyBool("maintenance4", static_cast<bool>(obj.maintenance4));
}
void DumpVkPhysicalDeviceVulkan13Properties(Printer &p, std::string name, const VkPhysicalDeviceVulkan13Properties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(77);
    p.PrintKeyValue("minSubgroupSize", obj.minSubgroupSize);
    p.PrintKeyValue("maxSubgroupSize", obj.maxSubgroupSize);
    p.PrintKeyValue("maxComputeWorkgroupSubgroups", obj.maxComputeWorkgroupSubgroups);
    DumpVkShaderStageFlags(p, "requiredSubgroupSizeStages", obj.requiredSubgroupSizeStages);
    p.PrintKeyValue("maxInlineUniformBlockSize", obj.maxInlineUniformBlockSize);
    p.PrintKeyValue("maxPerStageDescriptorInlineUniformBlocks", obj.maxPerStageDescriptorInlineUniformBlocks);
    p.PrintKeyValue("maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks", obj.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks);
    p.PrintKeyValue("maxDescriptorSetInlineUniformBlocks", obj.maxDescriptorSetInlineUniformBlocks);
    p.PrintKeyValue("maxDescriptorSetUpdateAfterBindInlineUniformBlocks", obj.maxDescriptorSetUpdateAfterBindInlineUniformBlocks);
    p.PrintKeyValue("maxInlineUniformTotalSize", obj.maxInlineUniformTotalSize);
    p.PrintKeyBool("integerDotProduct8BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct8BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct8BitSignedAccelerated", static_cast<bool>(obj.integerDotProduct8BitSignedAccelerated));
    p.PrintKeyBool("integerDotProduct8BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct8BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProduct4x8BitPackedUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct4x8BitPackedUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct4x8BitPackedSignedAccelerated", static_cast<bool>(obj.integerDotProduct4x8BitPackedSignedAccelerated));
    p.PrintKeyBool("integerDotProduct4x8BitPackedMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct4x8BitPackedMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProduct16BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct16BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct16BitSignedAccelerated", static_cast<bool>(obj.integerDotProduct16BitSignedAccelerated));
    p.PrintKeyBool("integerDotProduct16BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct16BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProduct32BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct32BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct32BitSignedAccelerated", static_cast<bool>(obj.integerDotProduct32BitSignedAccelerated));
    p.PrintKeyBool("integerDotProduct32BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct32BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProduct64BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProduct64BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProduct64BitSignedAccelerated", static_cast<bool>(obj.integerDotProduct64BitSignedAccelerated));
    p.PrintKeyBool("integerDotProduct64BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProduct64BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating8BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating8BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating8BitSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating8BitSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating8BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating8BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating4x8BitPackedUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating4x8BitPackedUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating4x8BitPackedSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating4x8BitPackedSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating4x8BitPackedMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating4x8BitPackedMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating16BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating16BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating16BitSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating16BitSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating16BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating16BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating32BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating32BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating32BitSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating32BitSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating32BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating32BitMixedSignednessAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating64BitUnsignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating64BitUnsignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating64BitSignedAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating64BitSignedAccelerated));
    p.PrintKeyBool("integerDotProductAccumulatingSaturating64BitMixedSignednessAccelerated", static_cast<bool>(obj.integerDotProductAccumulatingSaturating64BitMixedSignednessAccelerated));
    p.PrintKeyValue("storageTexelBufferOffsetAlignmentBytes", to_hex_str(p, obj.storageTexelBufferOffsetAlignmentBytes));
    p.PrintKeyBool("storageTexelBufferOffsetSingleTexelAlignment", static_cast<bool>(obj.storageTexelBufferOffsetSingleTexelAlignment));
    p.PrintKeyValue("uniformTexelBufferOffsetAlignmentBytes", to_hex_str(p, obj.uniformTexelBufferOffsetAlignmentBytes));
    p.PrintKeyBool("uniformTexelBufferOffsetSingleTexelAlignment", static_cast<bool>(obj.uniformTexelBufferOffsetSingleTexelAlignment));
    p.PrintKeyValue("maxBufferSize", to_hex_str(p, obj.maxBufferSize));
}
void DumpVkPhysicalDeviceVulkan14Features(Printer &p, std::string name, const VkPhysicalDeviceVulkan14Features &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(38);
    p.PrintKeyBool("globalPriorityQuery", static_cast<bool>(obj.globalPriorityQuery));
    p.PrintKeyBool("shaderSubgroupRotate", static_cast<bool>(obj.shaderSubgroupRotate));
    p.PrintKeyBool("shaderSubgroupRotateClustered", static_cast<bool>(obj.shaderSubgroupRotateClustered));
    p.PrintKeyBool("shaderFloatControls2", static_cast<bool>(obj.shaderFloatControls2));
    p.PrintKeyBool("shaderExpectAssume", static_cast<bool>(obj.shaderExpectAssume));
    p.PrintKeyBool("rectangularLines", static_cast<bool>(obj.rectangularLines));
    p.PrintKeyBool("bresenhamLines", static_cast<bool>(obj.bresenhamLines));
    p.PrintKeyBool("smoothLines", static_cast<bool>(obj.smoothLines));
    p.PrintKeyBool("stippledRectangularLines", static_cast<bool>(obj.stippledRectangularLines));
    p.PrintKeyBool("stippledBresenhamLines", static_cast<bool>(obj.stippledBresenhamLines));
    p.PrintKeyBool("stippledSmoothLines", static_cast<bool>(obj.stippledSmoothLines));
    p.PrintKeyBool("vertexAttributeInstanceRateDivisor", static_cast<bool>(obj.vertexAttributeInstanceRateDivisor));
    p.PrintKeyBool("vertexAttributeInstanceRateZeroDivisor", static_cast<bool>(obj.vertexAttributeInstanceRateZeroDivisor));
    p.PrintKeyBool("indexTypeUint8", static_cast<bool>(obj.indexTypeUint8));
    p.PrintKeyBool("dynamicRenderingLocalRead", static_cast<bool>(obj.dynamicRenderingLocalRead));
    p.PrintKeyBool("maintenance5", static_cast<bool>(obj.maintenance5));
    p.PrintKeyBool("maintenance6", static_cast<bool>(obj.maintenance6));
    p.PrintKeyBool("pipelineProtectedAccess", static_cast<bool>(obj.pipelineProtectedAccess));
    p.PrintKeyBool("pipelineRobustness", static_cast<bool>(obj.pipelineRobustness));
    p.PrintKeyBool("hostImageCopy", static_cast<bool>(obj.hostImageCopy));
    p.PrintKeyBool("pushDescriptor", static_cast<bool>(obj.pushDescriptor));
}
void DumpVkPhysicalDeviceVulkan14Properties(Printer &p, std::string name, const VkPhysicalDeviceVulkan14Properties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(51);
    p.PrintKeyValue("lineSubPixelPrecisionBits", obj.lineSubPixelPrecisionBits);
    p.PrintKeyValue("maxVertexAttribDivisor", obj.maxVertexAttribDivisor);
    p.PrintKeyBool("supportsNonZeroFirstInstance", static_cast<bool>(obj.supportsNonZeroFirstInstance));
    p.PrintKeyValue("maxPushDescriptors", obj.maxPushDescriptors);
    p.PrintKeyBool("dynamicRenderingLocalReadDepthStencilAttachments", static_cast<bool>(obj.dynamicRenderingLocalReadDepthStencilAttachments));
    p.PrintKeyBool("dynamicRenderingLocalReadMultisampledAttachments", static_cast<bool>(obj.dynamicRenderingLocalReadMultisampledAttachments));
    p.PrintKeyBool("earlyFragmentMultisampleCoverageAfterSampleCounting", static_cast<bool>(obj.earlyFragmentMultisampleCoverageAfterSampleCounting));
    p.PrintKeyBool("earlyFragmentSampleMaskTestBeforeSampleCounting", static_cast<bool>(obj.earlyFragmentSampleMaskTestBeforeSampleCounting));
    p.PrintKeyBool("depthStencilSwizzleOneSupport", static_cast<bool>(obj.depthStencilSwizzleOneSupport));
    p.PrintKeyBool("polygonModePointSize", static_cast<bool>(obj.polygonModePointSize));
    p.PrintKeyBool("nonStrictSinglePixelWideLinesUseParallelogram", static_cast<bool>(obj.nonStrictSinglePixelWideLinesUseParallelogram));
    p.PrintKeyBool("nonStrictWideLinesUseParallelogram", static_cast<bool>(obj.nonStrictWideLinesUseParallelogram));
    p.PrintKeyBool("blockTexelViewCompatibleMultipleLayers", static_cast<bool>(obj.blockTexelViewCompatibleMultipleLayers));
    p.PrintKeyValue("maxCombinedImageSamplerDescriptorCount", obj.maxCombinedImageSamplerDescriptorCount);
    p.PrintKeyBool("fragmentShadingRateClampCombinerInputs", static_cast<bool>(obj.fragmentShadingRateClampCombinerInputs));
    DumpVkPipelineRobustnessBufferBehavior(p, "defaultRobustnessStorageBuffers", obj.defaultRobustnessStorageBuffers);
    DumpVkPipelineRobustnessBufferBehavior(p, "defaultRobustnessUniformBuffers", obj.defaultRobustnessUniformBuffers);
    DumpVkPipelineRobustnessBufferBehavior(p, "defaultRobustnessVertexInputs", obj.defaultRobustnessVertexInputs);
    DumpVkPipelineRobustnessImageBehavior(p, "defaultRobustnessImages", obj.defaultRobustnessImages);
    p.PrintKeyValue("copySrcLayoutCount", obj.copySrcLayoutCount);
    if (obj.copySrcLayoutCount == 0 || obj.pCopySrcLayouts == nullptr) {
        p.PrintKeyString("pCopySrcLayouts", "NULL");
    } else {
        ArrayWrapper arr(p,"pCopySrcLayouts", obj.copySrcLayoutCount);
        for (uint32_t i = 0; i < obj.copySrcLayoutCount; i++) {
            DumpVkImageLayout(p, std::to_string(i), obj.pCopySrcLayouts[i]);
        }
    }
    p.PrintKeyValue("copyDstLayoutCount", obj.copyDstLayoutCount);
    if (obj.copyDstLayoutCount == 0 || obj.pCopyDstLayouts == nullptr) {
        p.PrintKeyString("pCopyDstLayouts", "NULL");
    } else {
        ArrayWrapper arr(p,"pCopyDstLayouts", obj.copyDstLayoutCount);
        for (uint32_t i = 0; i < obj.copyDstLayoutCount; i++) {
            DumpVkImageLayout(p, std::to_string(i), obj.pCopyDstLayouts[i]);
        }
    }
    p.PrintKeyValue("optimalTilingLayoutUUID", obj.optimalTilingLayoutUUID);
    p.PrintKeyBool("identicalMemoryTypeRequirements", static_cast<bool>(obj.identicalMemoryTypeRequirements));
}
void DumpVkPhysicalDeviceVulkanMemoryModelFeatures(Printer &p, std::string name, const VkPhysicalDeviceVulkanMemoryModelFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(45);
    p.PrintKeyBool("vulkanMemoryModel", static_cast<bool>(obj.vulkanMemoryModel));
    p.PrintKeyBool("vulkanMemoryModelDeviceScope", static_cast<bool>(obj.vulkanMemoryModelDeviceScope));
    p.PrintKeyBool("vulkanMemoryModelAvailabilityVisibilityChains", static_cast<bool>(obj.vulkanMemoryModelAvailabilityVisibilityChains));
}
void DumpVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(Printer &p, std::string name, const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(46);
    p.PrintKeyBool("workgroupMemoryExplicitLayout", static_cast<bool>(obj.workgroupMemoryExplicitLayout));
    p.PrintKeyBool("workgroupMemoryExplicitLayoutScalarBlockLayout", static_cast<bool>(obj.workgroupMemoryExplicitLayoutScalarBlockLayout));
    p.PrintKeyBool("workgroupMemoryExplicitLayout8BitAccess", static_cast<bool>(obj.workgroupMemoryExplicitLayout8BitAccess));
    p.PrintKeyBool("workgroupMemoryExplicitLayout16BitAccess", static_cast<bool>(obj.workgroupMemoryExplicitLayout16BitAccess));
}
void DumpVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(21);
    p.PrintKeyBool("ycbcr2plane444Formats", static_cast<bool>(obj.ycbcr2plane444Formats));
}
void DumpVkPhysicalDeviceYcbcrImageArraysFeaturesEXT(Printer &p, std::string name, const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    p.PrintKeyBool("ycbcrImageArrays", static_cast<bool>(obj.ycbcrImageArrays));
}
void DumpVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(Printer &p, std::string name, const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(35);
    p.PrintKeyBool("shaderZeroInitializeWorkgroupMemory", static_cast<bool>(obj.shaderZeroInitializeWorkgroupMemory));
}
void DumpVkQueueFamilyGlobalPriorityProperties(Printer &p, std::string name, const VkQueueFamilyGlobalPriorityProperties &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(14);
    p.PrintKeyValue("priorityCount", obj.priorityCount);
    ArrayWrapper arr(p,"priorities", obj.priorityCount);
    for (uint32_t i = 0; i < obj.priorityCount; i++) {
       if (p.Type() == OutputType::json)
           p.PrintString(std::string("VK_") + VkQueueGlobalPriorityString(obj.priorities[i]));
       else
           p.PrintString(VkQueueGlobalPriorityString(obj.priorities[i]));
    }
}
void DumpVkQueueFamilyQueryResultStatusPropertiesKHR(Printer &p, std::string name, const VkQueueFamilyQueryResultStatusPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(24);
    p.PrintKeyBool("queryResultStatusSupport", static_cast<bool>(obj.queryResultStatusSupport));
}
void DumpVkQueueFamilyVideoPropertiesKHR(Printer &p, std::string name, const VkQueueFamilyVideoPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkVideoCodecOperationFlagsKHR(p, "videoCodecOperations", obj.videoCodecOperations);
}
void DumpVkSharedPresentSurfaceCapabilitiesKHR(Printer &p, std::string name, const VkSharedPresentSurfaceCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkImageUsageFlags(p, "sharedPresentSupportedUsageFlags", obj.sharedPresentSupportedUsageFlags);
}
void DumpVkSubpassResolvePerformanceQueryEXT(Printer &p, std::string name, const VkSubpassResolvePerformanceQueryEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(7);
    p.PrintKeyBool("optimal", static_cast<bool>(obj.optimal));
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
void DumpVkSurfaceCapabilitiesFullScreenExclusiveEXT(Printer &p, std::string name, const VkSurfaceCapabilitiesFullScreenExclusiveEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(28);
    p.PrintKeyBool("fullScreenExclusiveSupported", static_cast<bool>(obj.fullScreenExclusiveSupported));
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
void DumpVkSurfaceCapabilitiesKHR(Printer &p, std::string name, const VkSurfaceCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(19);
    p.PrintKeyValue("minImageCount", obj.minImageCount);
    p.PrintKeyValue("maxImageCount", obj.maxImageCount);
    DumpVkExtent2D(p, "currentExtent", obj.currentExtent);
    DumpVkExtent2D(p, "minImageExtent", obj.minImageExtent);
    DumpVkExtent2D(p, "maxImageExtent", obj.maxImageExtent);
    p.PrintKeyValue("maxImageArrayLayers", obj.maxImageArrayLayers);
    DumpVkSurfaceTransformFlagsKHR(p, "supportedTransforms", obj.supportedTransforms);
    DumpVkSurfaceTransformFlagBitsKHR(p, "currentTransform", obj.currentTransform);
    DumpVkCompositeAlphaFlagsKHR(p, "supportedCompositeAlpha", obj.supportedCompositeAlpha);
    DumpVkImageUsageFlags(p, "supportedUsageFlags", obj.supportedUsageFlags);
}
void DumpVkSurfaceFormatKHR(Printer &p, std::string name, const VkSurfaceFormatKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkFormat(p, "format", obj.format);
    DumpVkColorSpaceKHR(p, "colorSpace", obj.colorSpace);
}
void DumpVkSurfacePresentModeCompatibilityEXT(Printer &p, std::string name, const VkSurfacePresentModeCompatibilityEXT &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(31);
    p.PrintKeyValue("presentModeCount", obj.presentModeCount);
    if (obj.presentModeCount == 0 || obj.pPresentModes == nullptr) {
        p.PrintKeyString("pPresentModes", "NULL");
    } else {
        ArrayWrapper arr(p,"pPresentModes", obj.presentModeCount);
        for (uint32_t i = 0; i < obj.presentModeCount; i++) {
            DumpVkPresentModeKHR(p, std::to_string(i), obj.pPresentModes[i]);
        }
    }
}
void DumpVkSurfacePresentScalingCapabilitiesEXT(Printer &p, std::string name, const VkSurfacePresentScalingCapabilitiesEXT &obj) {
    ObjectWrapper object{p, name};
    DumpVkPresentScalingFlagsEXT(p, "supportedPresentScaling", obj.supportedPresentScaling);
    DumpVkPresentGravityFlagsEXT(p, "supportedPresentGravityX", obj.supportedPresentGravityX);
    DumpVkPresentGravityFlagsEXT(p, "supportedPresentGravityY", obj.supportedPresentGravityY);
    DumpVkExtent2D(p, "minScaledImageExtent", obj.minScaledImageExtent);
    DumpVkExtent2D(p, "maxScaledImageExtent", obj.maxScaledImageExtent);
}
void DumpVkSurfaceProtectedCapabilitiesKHR(Printer &p, std::string name, const VkSurfaceProtectedCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(17);
    p.PrintKeyBool("supportsProtected", static_cast<bool>(obj.supportsProtected));
}
void DumpVkVideoCapabilitiesKHR(Printer &p, std::string name, const VkVideoCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(33);
    DumpVkVideoCapabilityFlagsKHR(p, "flags", obj.flags);
    p.PrintKeyValue("minBitstreamBufferOffsetAlignment", to_hex_str(p, obj.minBitstreamBufferOffsetAlignment));
    p.PrintKeyValue("minBitstreamBufferSizeAlignment", to_hex_str(p, obj.minBitstreamBufferSizeAlignment));
    DumpVkExtent2D(p, "pictureAccessGranularity", obj.pictureAccessGranularity);
    DumpVkExtent2D(p, "minCodedExtent", obj.minCodedExtent);
    DumpVkExtent2D(p, "maxCodedExtent", obj.maxCodedExtent);
    p.PrintKeyValue("maxDpbSlots", obj.maxDpbSlots);
    p.PrintKeyValue("maxActiveReferencePictures", obj.maxActiveReferencePictures);
    DumpVkExtensionProperties(p, "stdHeaderVersion", obj.stdHeaderVersion);
}
void DumpVkVideoDecodeAV1CapabilitiesKHR(Printer &p, std::string name, const VkVideoDecodeAV1CapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpStdVideoAV1Level(p, "maxLevel", obj.maxLevel);
}
void DumpVkVideoDecodeAV1ProfileInfoKHR(Printer &p, std::string name, const VkVideoDecodeAV1ProfileInfoKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(16);
    DumpStdVideoAV1Profile(p, "stdProfile", obj.stdProfile);
    p.PrintKeyBool("filmGrainSupport", static_cast<bool>(obj.filmGrainSupport));
}
void DumpVkVideoDecodeCapabilitiesKHR(Printer &p, std::string name, const VkVideoDecodeCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkVideoDecodeCapabilityFlagsKHR(p, "flags", obj.flags);
}
void DumpVkVideoDecodeH264CapabilitiesKHR(Printer &p, std::string name, const VkVideoDecodeH264CapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpStdVideoH264LevelIdc(p, "maxLevelIdc", obj.maxLevelIdc);
    DumpVkOffset2D(p, "fieldOffsetGranularity", obj.fieldOffsetGranularity);
}
void DumpVkVideoDecodeH264ProfileInfoKHR(Printer &p, std::string name, const VkVideoDecodeH264ProfileInfoKHR &obj) {
    ObjectWrapper object{p, name};
    DumpStdVideoH264ProfileIdc(p, "stdProfileIdc", obj.stdProfileIdc);
    DumpVkVideoDecodeH264PictureLayoutFlagBitsKHR(p, "pictureLayout", obj.pictureLayout);
}
void DumpVkVideoDecodeH265CapabilitiesKHR(Printer &p, std::string name, const VkVideoDecodeH265CapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpStdVideoH265LevelIdc(p, "maxLevelIdc", obj.maxLevelIdc);
}
void DumpVkVideoDecodeH265ProfileInfoKHR(Printer &p, std::string name, const VkVideoDecodeH265ProfileInfoKHR &obj) {
    ObjectWrapper object{p, name};
    DumpStdVideoH265ProfileIdc(p, "stdProfileIdc", obj.stdProfileIdc);
}
void DumpVkVideoDecodeUsageInfoKHR(Printer &p, std::string name, const VkVideoDecodeUsageInfoKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkVideoDecodeUsageFlagsKHR(p, "videoUsageHints", obj.videoUsageHints);
}
void DumpVkVideoEncodeAV1CapabilitiesKHR(Printer &p, std::string name, const VkVideoEncodeAV1CapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(45);
    DumpVkVideoEncodeAV1CapabilityFlagsKHR(p, "flags", obj.flags);
    DumpStdVideoAV1Level(p, "maxLevel", obj.maxLevel);
    DumpVkExtent2D(p, "codedPictureAlignment", obj.codedPictureAlignment);
    DumpVkExtent2D(p, "maxTiles", obj.maxTiles);
    DumpVkExtent2D(p, "minTileSize", obj.minTileSize);
    DumpVkExtent2D(p, "maxTileSize", obj.maxTileSize);
    DumpVkVideoEncodeAV1SuperblockSizeFlagsKHR(p, "superblockSizes", obj.superblockSizes);
    p.PrintKeyValue("maxSingleReferenceCount", obj.maxSingleReferenceCount);
    p.PrintKeyValue("singleReferenceNameMask", obj.singleReferenceNameMask);
    p.PrintKeyValue("maxUnidirectionalCompoundReferenceCount", obj.maxUnidirectionalCompoundReferenceCount);
    p.PrintKeyValue("maxUnidirectionalCompoundGroup1ReferenceCount", obj.maxUnidirectionalCompoundGroup1ReferenceCount);
    p.PrintKeyValue("unidirectionalCompoundReferenceNameMask", obj.unidirectionalCompoundReferenceNameMask);
    p.PrintKeyValue("maxBidirectionalCompoundReferenceCount", obj.maxBidirectionalCompoundReferenceCount);
    p.PrintKeyValue("maxBidirectionalCompoundGroup1ReferenceCount", obj.maxBidirectionalCompoundGroup1ReferenceCount);
    p.PrintKeyValue("maxBidirectionalCompoundGroup2ReferenceCount", obj.maxBidirectionalCompoundGroup2ReferenceCount);
    p.PrintKeyValue("bidirectionalCompoundReferenceNameMask", obj.bidirectionalCompoundReferenceNameMask);
    p.PrintKeyValue("maxTemporalLayerCount", obj.maxTemporalLayerCount);
    p.PrintKeyValue("maxSpatialLayerCount", obj.maxSpatialLayerCount);
    p.PrintKeyValue("maxOperatingPoints", obj.maxOperatingPoints);
    p.PrintKeyValue("minQIndex", obj.minQIndex);
    p.PrintKeyValue("maxQIndex", obj.maxQIndex);
    p.PrintKeyBool("prefersGopRemainingFrames", static_cast<bool>(obj.prefersGopRemainingFrames));
    p.PrintKeyBool("requiresGopRemainingFrames", static_cast<bool>(obj.requiresGopRemainingFrames));
    DumpVkVideoEncodeAV1StdFlagsKHR(p, "stdSyntaxFlags", obj.stdSyntaxFlags);
}
void DumpVkVideoEncodeAV1ProfileInfoKHR(Printer &p, std::string name, const VkVideoEncodeAV1ProfileInfoKHR &obj) {
    ObjectWrapper object{p, name};
    DumpStdVideoAV1Profile(p, "stdProfile", obj.stdProfile);
}
void DumpVkVideoEncodeAV1QuantizationMapCapabilitiesKHR(Printer &p, std::string name, const VkVideoEncodeAV1QuantizationMapCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(14);
    p.PrintKeyValue("minQIndexDelta", obj.minQIndexDelta);
    p.PrintKeyValue("maxQIndexDelta", obj.maxQIndexDelta);
}
void DumpVkVideoEncodeCapabilitiesKHR(Printer &p, std::string name, const VkVideoEncodeCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(20);
    DumpVkVideoEncodeCapabilityFlagsKHR(p, "flags", obj.flags);
    DumpVkVideoEncodeRateControlModeFlagsKHR(p, "rateControlModes", obj.rateControlModes);
    p.PrintKeyValue("maxRateControlLayers", obj.maxRateControlLayers);
    p.PrintKeyValue("maxBitrate", obj.maxBitrate);
    p.PrintKeyValue("maxQualityLevels", obj.maxQualityLevels);
    DumpVkExtent2D(p, "encodeInputPictureGranularity", obj.encodeInputPictureGranularity);
    DumpVkVideoEncodeFeedbackFlagsKHR(p, "supportedEncodeFeedbackFlags", obj.supportedEncodeFeedbackFlags);
}
void DumpVkVideoEncodeH264CapabilitiesKHR(Printer &p, std::string name, const VkVideoEncodeH264CapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(32);
    DumpVkVideoEncodeH264CapabilityFlagsKHR(p, "flags", obj.flags);
    DumpStdVideoH264LevelIdc(p, "maxLevelIdc", obj.maxLevelIdc);
    p.PrintKeyValue("maxSliceCount", obj.maxSliceCount);
    p.PrintKeyValue("maxPPictureL0ReferenceCount", obj.maxPPictureL0ReferenceCount);
    p.PrintKeyValue("maxBPictureL0ReferenceCount", obj.maxBPictureL0ReferenceCount);
    p.PrintKeyValue("maxL1ReferenceCount", obj.maxL1ReferenceCount);
    p.PrintKeyValue("maxTemporalLayerCount", obj.maxTemporalLayerCount);
    p.PrintKeyBool("expectDyadicTemporalLayerPattern", static_cast<bool>(obj.expectDyadicTemporalLayerPattern));
    p.PrintKeyValue("minQp", obj.minQp);
    p.PrintKeyValue("maxQp", obj.maxQp);
    p.PrintKeyBool("prefersGopRemainingFrames", static_cast<bool>(obj.prefersGopRemainingFrames));
    p.PrintKeyBool("requiresGopRemainingFrames", static_cast<bool>(obj.requiresGopRemainingFrames));
    DumpVkVideoEncodeH264StdFlagsKHR(p, "stdSyntaxFlags", obj.stdSyntaxFlags);
}
void DumpVkVideoEncodeH264ProfileInfoKHR(Printer &p, std::string name, const VkVideoEncodeH264ProfileInfoKHR &obj) {
    ObjectWrapper object{p, name};
    DumpStdVideoH264ProfileIdc(p, "stdProfileIdc", obj.stdProfileIdc);
}
void DumpVkVideoEncodeH264QuantizationMapCapabilitiesKHR(Printer &p, std::string name, const VkVideoEncodeH264QuantizationMapCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(10);
    p.PrintKeyValue("minQpDelta", obj.minQpDelta);
    p.PrintKeyValue("maxQpDelta", obj.maxQpDelta);
}
void DumpVkVideoEncodeH265CapabilitiesKHR(Printer &p, std::string name, const VkVideoEncodeH265CapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(35);
    DumpVkVideoEncodeH265CapabilityFlagsKHR(p, "flags", obj.flags);
    DumpStdVideoH265LevelIdc(p, "maxLevelIdc", obj.maxLevelIdc);
    p.PrintKeyValue("maxSliceSegmentCount", obj.maxSliceSegmentCount);
    DumpVkExtent2D(p, "maxTiles", obj.maxTiles);
    DumpVkVideoEncodeH265CtbSizeFlagsKHR(p, "ctbSizes", obj.ctbSizes);
    DumpVkVideoEncodeH265TransformBlockSizeFlagsKHR(p, "transformBlockSizes", obj.transformBlockSizes);
    p.PrintKeyValue("maxPPictureL0ReferenceCount", obj.maxPPictureL0ReferenceCount);
    p.PrintKeyValue("maxBPictureL0ReferenceCount", obj.maxBPictureL0ReferenceCount);
    p.PrintKeyValue("maxL1ReferenceCount", obj.maxL1ReferenceCount);
    p.PrintKeyValue("maxSubLayerCount", obj.maxSubLayerCount);
    p.PrintKeyBool("expectDyadicTemporalSubLayerPattern", static_cast<bool>(obj.expectDyadicTemporalSubLayerPattern));
    p.PrintKeyValue("minQp", obj.minQp);
    p.PrintKeyValue("maxQp", obj.maxQp);
    p.PrintKeyBool("prefersGopRemainingFrames", static_cast<bool>(obj.prefersGopRemainingFrames));
    p.PrintKeyBool("requiresGopRemainingFrames", static_cast<bool>(obj.requiresGopRemainingFrames));
    DumpVkVideoEncodeH265StdFlagsKHR(p, "stdSyntaxFlags", obj.stdSyntaxFlags);
}
void DumpVkVideoEncodeH265ProfileInfoKHR(Printer &p, std::string name, const VkVideoEncodeH265ProfileInfoKHR &obj) {
    ObjectWrapper object{p, name};
    DumpStdVideoH265ProfileIdc(p, "stdProfileIdc", obj.stdProfileIdc);
}
void DumpVkVideoEncodeH265QuantizationMapCapabilitiesKHR(Printer &p, std::string name, const VkVideoEncodeH265QuantizationMapCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    p.SetMinKeyWidth(10);
    p.PrintKeyValue("minQpDelta", obj.minQpDelta);
    p.PrintKeyValue("maxQpDelta", obj.maxQpDelta);
}
void DumpVkVideoEncodeQuantizationMapCapabilitiesKHR(Printer &p, std::string name, const VkVideoEncodeQuantizationMapCapabilitiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkExtent2D(p, "maxQuantizationMapExtent", obj.maxQuantizationMapExtent);
}
void DumpVkVideoEncodeUsageInfoKHR(Printer &p, std::string name, const VkVideoEncodeUsageInfoKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkVideoEncodeUsageFlagsKHR(p, "videoUsageHints", obj.videoUsageHints);
    DumpVkVideoEncodeContentFlagsKHR(p, "videoContentHints", obj.videoContentHints);
    DumpVkVideoEncodeTuningModeKHR(p, "tuningMode", obj.tuningMode);
}
void DumpVkVideoFormatAV1QuantizationMapPropertiesKHR(Printer &p, std::string name, const VkVideoFormatAV1QuantizationMapPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkVideoEncodeAV1SuperblockSizeFlagsKHR(p, "compatibleSuperblockSizes", obj.compatibleSuperblockSizes);
}
void DumpVkVideoFormatH265QuantizationMapPropertiesKHR(Printer &p, std::string name, const VkVideoFormatH265QuantizationMapPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkVideoEncodeH265CtbSizeFlagsKHR(p, "compatibleCtbSizes", obj.compatibleCtbSizes);
}
void DumpVkVideoFormatPropertiesKHR(Printer &p, std::string name, const VkVideoFormatPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkFormat(p, "format", obj.format);
    DumpVkComponentMapping(p, "componentMapping", obj.componentMapping);
    DumpVkImageCreateFlags(p, "imageCreateFlags", obj.imageCreateFlags);
    DumpVkImageType(p, "imageType", obj.imageType);
    DumpVkImageTiling(p, "imageTiling", obj.imageTiling);
    DumpVkImageUsageFlags(p, "imageUsageFlags", obj.imageUsageFlags);
}
void DumpVkVideoFormatQuantizationMapPropertiesKHR(Printer &p, std::string name, const VkVideoFormatQuantizationMapPropertiesKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkExtent2D(p, "quantizationMapTexelSize", obj.quantizationMapTexelSize);
}
void DumpVkVideoProfileInfoKHR(Printer &p, std::string name, const VkVideoProfileInfoKHR &obj) {
    ObjectWrapper object{p, name};
    DumpVkVideoCodecOperationFlagBitsKHR(p, "videoCodecOperation", obj.videoCodecOperation);
    DumpVkVideoChromaSubsamplingFlagsKHR(p, "chromaSubsampling", obj.chromaSubsampling);
    DumpVkVideoComponentBitDepthFlagsKHR(p, "lumaBitDepth", obj.lumaBitDepth);
    DumpVkVideoComponentBitDepthFlagsKHR(p, "chromaBitDepth", obj.chromaBitDepth);
}
struct phys_device_props2_chain {
    phys_device_props2_chain() = default;
    phys_device_props2_chain(const phys_device_props2_chain &) = delete;
    phys_device_props2_chain& operator=(const phys_device_props2_chain &) = delete;
    phys_device_props2_chain(phys_device_props2_chain &&) = delete;
    phys_device_props2_chain& operator=(phys_device_props2_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkPhysicalDeviceAccelerationStructurePropertiesKHR PhysicalDeviceAccelerationStructurePropertiesKHR{};
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT PhysicalDeviceBlendOperationAdvancedPropertiesEXT{};
    VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR PhysicalDeviceComputeShaderDerivativesPropertiesKHR{};
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT PhysicalDeviceConservativeRasterizationPropertiesEXT{};
    VkPhysicalDeviceCooperativeMatrixPropertiesKHR PhysicalDeviceCooperativeMatrixPropertiesKHR{};
    VkPhysicalDeviceCustomBorderColorPropertiesEXT PhysicalDeviceCustomBorderColorPropertiesEXT{};
    VkPhysicalDeviceDepthStencilResolveProperties PhysicalDeviceDepthStencilResolveProperties{};
    VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT PhysicalDeviceDescriptorBufferDensityMapPropertiesEXT{};
    VkPhysicalDeviceDescriptorBufferPropertiesEXT PhysicalDeviceDescriptorBufferPropertiesEXT{};
    VkPhysicalDeviceDescriptorIndexingProperties PhysicalDeviceDescriptorIndexingProperties{};
    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT PhysicalDeviceDeviceGeneratedCommandsPropertiesEXT{};
    VkPhysicalDeviceDiscardRectanglePropertiesEXT PhysicalDeviceDiscardRectanglePropertiesEXT{};
    VkPhysicalDeviceDriverProperties PhysicalDeviceDriverProperties{};
    VkPhysicalDeviceDrmPropertiesEXT PhysicalDeviceDrmPropertiesEXT{};
    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT PhysicalDeviceExtendedDynamicState3PropertiesEXT{};
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT PhysicalDeviceExternalMemoryHostPropertiesEXT{};
    VkPhysicalDeviceFloatControlsProperties PhysicalDeviceFloatControlsProperties{};
    VkPhysicalDeviceFragmentDensityMap2PropertiesEXT PhysicalDeviceFragmentDensityMap2PropertiesEXT{};
    VkPhysicalDeviceFragmentDensityMapPropertiesEXT PhysicalDeviceFragmentDensityMapPropertiesEXT{};
    VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR PhysicalDeviceFragmentShaderBarycentricPropertiesKHR{};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR PhysicalDeviceFragmentShadingRatePropertiesKHR{};
    VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT PhysicalDeviceGraphicsPipelineLibraryPropertiesEXT{};
    VkPhysicalDeviceHostImageCopyProperties PhysicalDeviceHostImageCopyProperties{};
    std::vector<VkImageLayout> VkPhysicalDeviceHostImageCopyProperties_pCopySrcLayouts;
    std::vector<VkImageLayout> VkPhysicalDeviceHostImageCopyProperties_pCopyDstLayouts;
    VkPhysicalDeviceIDProperties PhysicalDeviceIDProperties{};
    VkPhysicalDeviceInlineUniformBlockProperties PhysicalDeviceInlineUniformBlockProperties{};
    VkPhysicalDeviceLayeredApiPropertiesListKHR PhysicalDeviceLayeredApiPropertiesListKHR{};
    std::vector<VkPhysicalDeviceLayeredApiPropertiesKHR> VkPhysicalDeviceLayeredApiPropertiesListKHR_pLayeredApis;
    VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT PhysicalDeviceLegacyVertexAttributesPropertiesEXT{};
    VkPhysicalDeviceLineRasterizationProperties PhysicalDeviceLineRasterizationProperties{};
    VkPhysicalDeviceMaintenance3Properties PhysicalDeviceMaintenance3Properties{};
    VkPhysicalDeviceMaintenance4Properties PhysicalDeviceMaintenance4Properties{};
    VkPhysicalDeviceMaintenance5Properties PhysicalDeviceMaintenance5Properties{};
    VkPhysicalDeviceMaintenance6Properties PhysicalDeviceMaintenance6Properties{};
    VkPhysicalDeviceMaintenance7PropertiesKHR PhysicalDeviceMaintenance7PropertiesKHR{};
    VkPhysicalDeviceMapMemoryPlacedPropertiesEXT PhysicalDeviceMapMemoryPlacedPropertiesEXT{};
    VkPhysicalDeviceMeshShaderPropertiesEXT PhysicalDeviceMeshShaderPropertiesEXT{};
    VkPhysicalDeviceMultiDrawPropertiesEXT PhysicalDeviceMultiDrawPropertiesEXT{};
    VkPhysicalDeviceMultiviewProperties PhysicalDeviceMultiviewProperties{};
    VkPhysicalDeviceNestedCommandBufferPropertiesEXT PhysicalDeviceNestedCommandBufferPropertiesEXT{};
    VkPhysicalDeviceOpacityMicromapPropertiesEXT PhysicalDeviceOpacityMicromapPropertiesEXT{};
    VkPhysicalDevicePCIBusInfoPropertiesEXT PhysicalDevicePCIBusInfoPropertiesEXT{};
    VkPhysicalDevicePerformanceQueryPropertiesKHR PhysicalDevicePerformanceQueryPropertiesKHR{};
    VkPhysicalDevicePipelineBinaryPropertiesKHR PhysicalDevicePipelineBinaryPropertiesKHR{};
    VkPhysicalDevicePipelineRobustnessProperties PhysicalDevicePipelineRobustnessProperties{};
    VkPhysicalDevicePointClippingProperties PhysicalDevicePointClippingProperties{};
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VkPhysicalDevicePortabilitySubsetPropertiesKHR PhysicalDevicePortabilitySubsetPropertiesKHR{};
#endif  // VK_ENABLE_BETA_EXTENSIONS
    VkPhysicalDeviceProtectedMemoryProperties PhysicalDeviceProtectedMemoryProperties{};
    VkPhysicalDeviceProvokingVertexPropertiesEXT PhysicalDeviceProvokingVertexPropertiesEXT{};
    VkPhysicalDevicePushDescriptorProperties PhysicalDevicePushDescriptorProperties{};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR PhysicalDeviceRayTracingPipelinePropertiesKHR{};
    VkPhysicalDeviceRobustness2PropertiesEXT PhysicalDeviceRobustness2PropertiesEXT{};
    VkPhysicalDeviceSampleLocationsPropertiesEXT PhysicalDeviceSampleLocationsPropertiesEXT{};
    VkPhysicalDeviceSamplerFilterMinmaxProperties PhysicalDeviceSamplerFilterMinmaxProperties{};
    VkPhysicalDeviceShaderIntegerDotProductProperties PhysicalDeviceShaderIntegerDotProductProperties{};
    VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT PhysicalDeviceShaderModuleIdentifierPropertiesEXT{};
    VkPhysicalDeviceShaderObjectPropertiesEXT PhysicalDeviceShaderObjectPropertiesEXT{};
    VkPhysicalDeviceShaderTileImagePropertiesEXT PhysicalDeviceShaderTileImagePropertiesEXT{};
    VkPhysicalDeviceSubgroupProperties PhysicalDeviceSubgroupProperties{};
    VkPhysicalDeviceSubgroupSizeControlProperties PhysicalDeviceSubgroupSizeControlProperties{};
    VkPhysicalDeviceTexelBufferAlignmentProperties PhysicalDeviceTexelBufferAlignmentProperties{};
    VkPhysicalDeviceTimelineSemaphoreProperties PhysicalDeviceTimelineSemaphoreProperties{};
    VkPhysicalDeviceTransformFeedbackPropertiesEXT PhysicalDeviceTransformFeedbackPropertiesEXT{};
    VkPhysicalDeviceVertexAttributeDivisorProperties PhysicalDeviceVertexAttributeDivisorProperties{};
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT PhysicalDeviceVertexAttributeDivisorPropertiesEXT{};
    VkPhysicalDeviceVulkan11Properties PhysicalDeviceVulkan11Properties{};
    VkPhysicalDeviceVulkan12Properties PhysicalDeviceVulkan12Properties{};
    VkPhysicalDeviceVulkan13Properties PhysicalDeviceVulkan13Properties{};
    VkPhysicalDeviceVulkan14Properties PhysicalDeviceVulkan14Properties{};
    std::vector<VkImageLayout> VkPhysicalDeviceVulkan14Properties_pCopySrcLayouts;
    std::vector<VkImageLayout> VkPhysicalDeviceVulkan14Properties_pCopyDstLayouts;
    void initialize_chain(AppInstance &inst, AppGpu &gpu , bool show_promoted_structs) noexcept {
        PhysicalDeviceAccelerationStructurePropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
        PhysicalDeviceBlendOperationAdvancedPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT;
        PhysicalDeviceComputeShaderDerivativesPropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR;
        PhysicalDeviceConservativeRasterizationPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
        PhysicalDeviceCooperativeMatrixPropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR;
        PhysicalDeviceCustomBorderColorPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT;
        PhysicalDeviceDepthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
        PhysicalDeviceDescriptorBufferDensityMapPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT;
        PhysicalDeviceDescriptorBufferPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
        PhysicalDeviceDescriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
        PhysicalDeviceDeviceGeneratedCommandsPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT;
        PhysicalDeviceDiscardRectanglePropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT;
        PhysicalDeviceDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
        PhysicalDeviceDrmPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT;
        PhysicalDeviceExtendedDynamicState3PropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
        PhysicalDeviceExternalMemoryHostPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT;
        PhysicalDeviceFloatControlsProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES;
        PhysicalDeviceFragmentDensityMap2PropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT;
        PhysicalDeviceFragmentDensityMapPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT;
        PhysicalDeviceFragmentShaderBarycentricPropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR;
        PhysicalDeviceFragmentShadingRatePropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
        PhysicalDeviceGraphicsPipelineLibraryPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT;
        PhysicalDeviceHostImageCopyProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES;
        PhysicalDeviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        PhysicalDeviceInlineUniformBlockProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES;
        PhysicalDeviceLayeredApiPropertiesListKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR;
        PhysicalDeviceLegacyVertexAttributesPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT;
        PhysicalDeviceLineRasterizationProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES;
        PhysicalDeviceMaintenance3Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
        PhysicalDeviceMaintenance4Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES;
        PhysicalDeviceMaintenance5Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES;
        PhysicalDeviceMaintenance6Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES;
        PhysicalDeviceMaintenance7PropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR;
        PhysicalDeviceMapMemoryPlacedPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT;
        PhysicalDeviceMeshShaderPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
        PhysicalDeviceMultiDrawPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT;
        PhysicalDeviceMultiviewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
        PhysicalDeviceNestedCommandBufferPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT;
        PhysicalDeviceOpacityMicromapPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT;
        PhysicalDevicePCIBusInfoPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
        PhysicalDevicePerformanceQueryPropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR;
        PhysicalDevicePipelineBinaryPropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR;
        PhysicalDevicePipelineRobustnessProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES;
        PhysicalDevicePointClippingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        PhysicalDevicePortabilitySubsetPropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        PhysicalDeviceProtectedMemoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES;
        PhysicalDeviceProvokingVertexPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT;
        PhysicalDevicePushDescriptorProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES;
        PhysicalDeviceRayTracingPipelinePropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        PhysicalDeviceRobustness2PropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT;
        PhysicalDeviceSampleLocationsPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT;
        PhysicalDeviceSamplerFilterMinmaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
        PhysicalDeviceShaderIntegerDotProductProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES;
        PhysicalDeviceShaderModuleIdentifierPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT;
        PhysicalDeviceShaderObjectPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT;
        PhysicalDeviceShaderTileImagePropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT;
        PhysicalDeviceSubgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
        PhysicalDeviceSubgroupSizeControlProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES;
        PhysicalDeviceTexelBufferAlignmentProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES;
        PhysicalDeviceTimelineSemaphoreProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES;
        PhysicalDeviceTransformFeedbackPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT;
        PhysicalDeviceVertexAttributeDivisorProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES;
        PhysicalDeviceVertexAttributeDivisorPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT;
        PhysicalDeviceVulkan11Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        PhysicalDeviceVulkan12Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
        PhysicalDeviceVulkan13Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
        PhysicalDeviceVulkan14Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES;
        std::vector<VkBaseOutStructure*> chain_members{};
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceAccelerationStructurePropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceBlendOperationAdvancedPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceComputeShaderDerivativesPropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceConservativeRasterizationPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceCooperativeMatrixPropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceCustomBorderColorPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDepthStencilResolveProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDescriptorBufferDensityMapPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDescriptorBufferPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDescriptorIndexingProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDeviceGeneratedCommandsPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDiscardRectanglePropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDriverProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDrmPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceExtendedDynamicState3PropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceExternalMemoryHostPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFloatControlsProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentDensityMap2PropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentDensityMapPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentShaderBarycentricPropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentShadingRatePropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceGraphicsPipelineLibraryPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceHostImageCopyProperties));
        if ((inst.CheckExtensionEnabled(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)
         || inst.CheckExtensionEnabled(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME)
         || inst.CheckExtensionEnabled(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_1 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceIDProperties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceInlineUniformBlockProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_7_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceLayeredApiPropertiesListKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_LEGACY_VERTEX_ATTRIBUTES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceLegacyVertexAttributesPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceLineRasterizationProperties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_3_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_1 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance3Properties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_4_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance4Properties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_5_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance5Properties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_6_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance6Properties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_7_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance7PropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MAP_MEMORY_PLACED_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMapMemoryPlacedPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MESH_SHADER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMeshShaderPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MULTI_DRAW_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMultiDrawPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MULTIVIEW_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_1 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMultiviewProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceNestedCommandBufferPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceOpacityMicromapPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PCI_BUS_INFO_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePCIBusInfoPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePerformanceQueryPropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelineBinaryPropertiesKHR));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelineRobustnessProperties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_2_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_1 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePointClippingProperties));
#ifdef VK_ENABLE_BETA_EXTENSIONS
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePortabilitySubsetPropertiesKHR));
#endif  // VK_ENABLE_BETA_EXTENSIONS
        if (gpu.api_version == VK_API_VERSION_1_1 || show_promoted_structs)
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceProtectedMemoryProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceProvokingVertexPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePushDescriptorProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRayTracingPipelinePropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRobustness2PropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSampleLocationsPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSamplerFilterMinmaxProperties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderIntegerDotProductProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderModuleIdentifierPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_OBJECT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderObjectPropertiesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderTileImagePropertiesEXT));
        if (gpu.api_version == VK_API_VERSION_1_1 || show_promoted_structs)
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSubgroupProperties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSubgroupSizeControlProperties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceTexelBufferAlignmentProperties));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceTimelineSemaphoreProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceTransformFeedbackPropertiesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVertexAttributeDivisorProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVertexAttributeDivisorPropertiesEXT));
        if ((gpu.api_version >= VK_API_VERSION_1_2))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkan11Properties));
        if ((gpu.api_version >= VK_API_VERSION_1_2))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkan12Properties));
        if ((gpu.api_version >= VK_API_VERSION_1_3))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkan13Properties));
        if ((gpu.api_version >= VK_API_VERSION_1_4))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkan14Properties));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_phys_device_props2_chain(VkPhysicalDeviceProperties2& start, std::unique_ptr<phys_device_props2_chain>& chain, AppInstance &inst,AppGpu &gpu,bool show_promoted_structs){
    chain = std::unique_ptr<phys_device_props2_chain>(new phys_device_props2_chain());
    chain->initialize_chain(inst,gpu,show_promoted_structs);
    start.pNext = chain->start_of_chain;
};

void chain_iterator_phys_device_props2(Printer &p, AppInstance &inst, AppGpu &gpu, bool show_promoted_structs, const void * place) {
    while (place) {
        const VkBaseOutStructure *structure = (const VkBaseOutStructure *)place;
        p.SetSubHeader();
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR) {
            const VkPhysicalDeviceAccelerationStructurePropertiesKHR* props = (const VkPhysicalDeviceAccelerationStructurePropertiesKHR*)structure;
            DumpVkPhysicalDeviceAccelerationStructurePropertiesKHR(p, "VkPhysicalDeviceAccelerationStructurePropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT) {
            const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* props = (const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT*)structure;
            DumpVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(p, "VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR) {
            const VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR* props = (const VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR*)structure;
            DumpVkPhysicalDeviceComputeShaderDerivativesPropertiesKHR(p, "VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT) {
            const VkPhysicalDeviceConservativeRasterizationPropertiesEXT* props = (const VkPhysicalDeviceConservativeRasterizationPropertiesEXT*)structure;
            DumpVkPhysicalDeviceConservativeRasterizationPropertiesEXT(p, "VkPhysicalDeviceConservativeRasterizationPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR) {
            const VkPhysicalDeviceCooperativeMatrixPropertiesKHR* props = (const VkPhysicalDeviceCooperativeMatrixPropertiesKHR*)structure;
            DumpVkPhysicalDeviceCooperativeMatrixPropertiesKHR(p, "VkPhysicalDeviceCooperativeMatrixPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT) {
            const VkPhysicalDeviceCustomBorderColorPropertiesEXT* props = (const VkPhysicalDeviceCustomBorderColorPropertiesEXT*)structure;
            DumpVkPhysicalDeviceCustomBorderColorPropertiesEXT(p, "VkPhysicalDeviceCustomBorderColorPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES) {
            const VkPhysicalDeviceDepthStencilResolveProperties* props = (const VkPhysicalDeviceDepthStencilResolveProperties*)structure;
            DumpVkPhysicalDeviceDepthStencilResolveProperties(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceDepthStencilResolveProperties":"VkPhysicalDeviceDepthStencilResolvePropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT) {
            const VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT* props = (const VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT*)structure;
            DumpVkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT(p, "VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT) {
            const VkPhysicalDeviceDescriptorBufferPropertiesEXT* props = (const VkPhysicalDeviceDescriptorBufferPropertiesEXT*)structure;
            DumpVkPhysicalDeviceDescriptorBufferPropertiesEXT(p, "VkPhysicalDeviceDescriptorBufferPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES) {
            const VkPhysicalDeviceDescriptorIndexingProperties* props = (const VkPhysicalDeviceDescriptorIndexingProperties*)structure;
            DumpVkPhysicalDeviceDescriptorIndexingProperties(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceDescriptorIndexingProperties":"VkPhysicalDeviceDescriptorIndexingPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT) {
            const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT* props = (const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT*)structure;
            DumpVkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT(p, "VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT) {
            const VkPhysicalDeviceDiscardRectanglePropertiesEXT* props = (const VkPhysicalDeviceDiscardRectanglePropertiesEXT*)structure;
            DumpVkPhysicalDeviceDiscardRectanglePropertiesEXT(p, "VkPhysicalDeviceDiscardRectanglePropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES) {
            const VkPhysicalDeviceDriverProperties* props = (const VkPhysicalDeviceDriverProperties*)structure;
            DumpVkPhysicalDeviceDriverProperties(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceDriverProperties":"VkPhysicalDeviceDriverPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT) {
            const VkPhysicalDeviceDrmPropertiesEXT* props = (const VkPhysicalDeviceDrmPropertiesEXT*)structure;
            DumpVkPhysicalDeviceDrmPropertiesEXT(p, "VkPhysicalDeviceDrmPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT) {
            const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT* props = (const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT*)structure;
            DumpVkPhysicalDeviceExtendedDynamicState3PropertiesEXT(p, "VkPhysicalDeviceExtendedDynamicState3PropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT) {
            const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* props = (const VkPhysicalDeviceExternalMemoryHostPropertiesEXT*)structure;
            DumpVkPhysicalDeviceExternalMemoryHostPropertiesEXT(p, "VkPhysicalDeviceExternalMemoryHostPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES) {
            const VkPhysicalDeviceFloatControlsProperties* props = (const VkPhysicalDeviceFloatControlsProperties*)structure;
            DumpVkPhysicalDeviceFloatControlsProperties(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceFloatControlsProperties":"VkPhysicalDeviceFloatControlsPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT) {
            const VkPhysicalDeviceFragmentDensityMap2PropertiesEXT* props = (const VkPhysicalDeviceFragmentDensityMap2PropertiesEXT*)structure;
            DumpVkPhysicalDeviceFragmentDensityMap2PropertiesEXT(p, "VkPhysicalDeviceFragmentDensityMap2PropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT) {
            const VkPhysicalDeviceFragmentDensityMapPropertiesEXT* props = (const VkPhysicalDeviceFragmentDensityMapPropertiesEXT*)structure;
            DumpVkPhysicalDeviceFragmentDensityMapPropertiesEXT(p, "VkPhysicalDeviceFragmentDensityMapPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR) {
            const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* props = (const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR*)structure;
            DumpVkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(p, "VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR) {
            const VkPhysicalDeviceFragmentShadingRatePropertiesKHR* props = (const VkPhysicalDeviceFragmentShadingRatePropertiesKHR*)structure;
            DumpVkPhysicalDeviceFragmentShadingRatePropertiesKHR(p, "VkPhysicalDeviceFragmentShadingRatePropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT) {
            const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT* props = (const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT*)structure;
            DumpVkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(p, "VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES) {
            const VkPhysicalDeviceHostImageCopyProperties* props = (const VkPhysicalDeviceHostImageCopyProperties*)structure;
            DumpVkPhysicalDeviceHostImageCopyProperties(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceHostImageCopyProperties":"VkPhysicalDeviceHostImageCopyPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES) {
            const VkPhysicalDeviceIDProperties* props = (const VkPhysicalDeviceIDProperties*)structure;
            DumpVkPhysicalDeviceIDProperties(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDeviceIDProperties":"VkPhysicalDeviceIDPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES) {
            const VkPhysicalDeviceInlineUniformBlockProperties* props = (const VkPhysicalDeviceInlineUniformBlockProperties*)structure;
            DumpVkPhysicalDeviceInlineUniformBlockProperties(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceInlineUniformBlockProperties":"VkPhysicalDeviceInlineUniformBlockPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR) {
            const VkPhysicalDeviceLayeredApiPropertiesListKHR* props = (const VkPhysicalDeviceLayeredApiPropertiesListKHR*)structure;
            DumpVkPhysicalDeviceLayeredApiPropertiesListKHR(p, "VkPhysicalDeviceLayeredApiPropertiesListKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT) {
            const VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT* props = (const VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT*)structure;
            DumpVkPhysicalDeviceLegacyVertexAttributesPropertiesEXT(p, "VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES) {
            const VkPhysicalDeviceLineRasterizationProperties* props = (const VkPhysicalDeviceLineRasterizationProperties*)structure;
            DumpVkPhysicalDeviceLineRasterizationProperties(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceLineRasterizationProperties":"VkPhysicalDeviceLineRasterizationPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES) {
            const VkPhysicalDeviceMaintenance3Properties* props = (const VkPhysicalDeviceMaintenance3Properties*)structure;
            DumpVkPhysicalDeviceMaintenance3Properties(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDeviceMaintenance3Properties":"VkPhysicalDeviceMaintenance3PropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES) {
            const VkPhysicalDeviceMaintenance4Properties* props = (const VkPhysicalDeviceMaintenance4Properties*)structure;
            DumpVkPhysicalDeviceMaintenance4Properties(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceMaintenance4Properties":"VkPhysicalDeviceMaintenance4PropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES) {
            const VkPhysicalDeviceMaintenance5Properties* props = (const VkPhysicalDeviceMaintenance5Properties*)structure;
            DumpVkPhysicalDeviceMaintenance5Properties(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceMaintenance5Properties":"VkPhysicalDeviceMaintenance5PropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES) {
            const VkPhysicalDeviceMaintenance6Properties* props = (const VkPhysicalDeviceMaintenance6Properties*)structure;
            DumpVkPhysicalDeviceMaintenance6Properties(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceMaintenance6Properties":"VkPhysicalDeviceMaintenance6PropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR) {
            const VkPhysicalDeviceMaintenance7PropertiesKHR* props = (const VkPhysicalDeviceMaintenance7PropertiesKHR*)structure;
            DumpVkPhysicalDeviceMaintenance7PropertiesKHR(p, "VkPhysicalDeviceMaintenance7PropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT) {
            const VkPhysicalDeviceMapMemoryPlacedPropertiesEXT* props = (const VkPhysicalDeviceMapMemoryPlacedPropertiesEXT*)structure;
            DumpVkPhysicalDeviceMapMemoryPlacedPropertiesEXT(p, "VkPhysicalDeviceMapMemoryPlacedPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT) {
            const VkPhysicalDeviceMeshShaderPropertiesEXT* props = (const VkPhysicalDeviceMeshShaderPropertiesEXT*)structure;
            DumpVkPhysicalDeviceMeshShaderPropertiesEXT(p, "VkPhysicalDeviceMeshShaderPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT) {
            const VkPhysicalDeviceMultiDrawPropertiesEXT* props = (const VkPhysicalDeviceMultiDrawPropertiesEXT*)structure;
            DumpVkPhysicalDeviceMultiDrawPropertiesEXT(p, "VkPhysicalDeviceMultiDrawPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES) {
            const VkPhysicalDeviceMultiviewProperties* props = (const VkPhysicalDeviceMultiviewProperties*)structure;
            DumpVkPhysicalDeviceMultiviewProperties(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDeviceMultiviewProperties":"VkPhysicalDeviceMultiviewPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT) {
            const VkPhysicalDeviceNestedCommandBufferPropertiesEXT* props = (const VkPhysicalDeviceNestedCommandBufferPropertiesEXT*)structure;
            DumpVkPhysicalDeviceNestedCommandBufferPropertiesEXT(p, "VkPhysicalDeviceNestedCommandBufferPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT) {
            const VkPhysicalDeviceOpacityMicromapPropertiesEXT* props = (const VkPhysicalDeviceOpacityMicromapPropertiesEXT*)structure;
            DumpVkPhysicalDeviceOpacityMicromapPropertiesEXT(p, "VkPhysicalDeviceOpacityMicromapPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT) {
            const VkPhysicalDevicePCIBusInfoPropertiesEXT* props = (const VkPhysicalDevicePCIBusInfoPropertiesEXT*)structure;
            DumpVkPhysicalDevicePCIBusInfoPropertiesEXT(p, "VkPhysicalDevicePCIBusInfoPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR) {
            const VkPhysicalDevicePerformanceQueryPropertiesKHR* props = (const VkPhysicalDevicePerformanceQueryPropertiesKHR*)structure;
            DumpVkPhysicalDevicePerformanceQueryPropertiesKHR(p, "VkPhysicalDevicePerformanceQueryPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR) {
            const VkPhysicalDevicePipelineBinaryPropertiesKHR* props = (const VkPhysicalDevicePipelineBinaryPropertiesKHR*)structure;
            DumpVkPhysicalDevicePipelineBinaryPropertiesKHR(p, "VkPhysicalDevicePipelineBinaryPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES) {
            const VkPhysicalDevicePipelineRobustnessProperties* props = (const VkPhysicalDevicePipelineRobustnessProperties*)structure;
            DumpVkPhysicalDevicePipelineRobustnessProperties(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDevicePipelineRobustnessProperties":"VkPhysicalDevicePipelineRobustnessPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES) {
            const VkPhysicalDevicePointClippingProperties* props = (const VkPhysicalDevicePointClippingProperties*)structure;
            DumpVkPhysicalDevicePointClippingProperties(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDevicePointClippingProperties":"VkPhysicalDevicePointClippingPropertiesKHR", *props);
            p.AddNewline();
        }
#ifdef VK_ENABLE_BETA_EXTENSIONS
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR && p.Type() != OutputType::json) {
            const VkPhysicalDevicePortabilitySubsetPropertiesKHR* props = (const VkPhysicalDevicePortabilitySubsetPropertiesKHR*)structure;
            DumpVkPhysicalDevicePortabilitySubsetPropertiesKHR(p, "VkPhysicalDevicePortabilitySubsetPropertiesKHR", *props);
            p.AddNewline();
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES) {
            const VkPhysicalDeviceProtectedMemoryProperties* props = (const VkPhysicalDeviceProtectedMemoryProperties*)structure;
            DumpVkPhysicalDeviceProtectedMemoryProperties(p, "VkPhysicalDeviceProtectedMemoryProperties", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT) {
            const VkPhysicalDeviceProvokingVertexPropertiesEXT* props = (const VkPhysicalDeviceProvokingVertexPropertiesEXT*)structure;
            DumpVkPhysicalDeviceProvokingVertexPropertiesEXT(p, "VkPhysicalDeviceProvokingVertexPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES) {
            const VkPhysicalDevicePushDescriptorProperties* props = (const VkPhysicalDevicePushDescriptorProperties*)structure;
            DumpVkPhysicalDevicePushDescriptorProperties(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDevicePushDescriptorProperties":"VkPhysicalDevicePushDescriptorPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR) {
            const VkPhysicalDeviceRayTracingPipelinePropertiesKHR* props = (const VkPhysicalDeviceRayTracingPipelinePropertiesKHR*)structure;
            DumpVkPhysicalDeviceRayTracingPipelinePropertiesKHR(p, "VkPhysicalDeviceRayTracingPipelinePropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT) {
            const VkPhysicalDeviceRobustness2PropertiesEXT* props = (const VkPhysicalDeviceRobustness2PropertiesEXT*)structure;
            DumpVkPhysicalDeviceRobustness2PropertiesEXT(p, "VkPhysicalDeviceRobustness2PropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT) {
            const VkPhysicalDeviceSampleLocationsPropertiesEXT* props = (const VkPhysicalDeviceSampleLocationsPropertiesEXT*)structure;
            DumpVkPhysicalDeviceSampleLocationsPropertiesEXT(p, "VkPhysicalDeviceSampleLocationsPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES) {
            const VkPhysicalDeviceSamplerFilterMinmaxProperties* props = (const VkPhysicalDeviceSamplerFilterMinmaxProperties*)structure;
            DumpVkPhysicalDeviceSamplerFilterMinmaxProperties(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceSamplerFilterMinmaxProperties":"VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES) {
            const VkPhysicalDeviceShaderIntegerDotProductProperties* props = (const VkPhysicalDeviceShaderIntegerDotProductProperties*)structure;
            DumpVkPhysicalDeviceShaderIntegerDotProductProperties(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceShaderIntegerDotProductProperties":"VkPhysicalDeviceShaderIntegerDotProductPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT) {
            const VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT* props = (const VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT*)structure;
            DumpVkPhysicalDeviceShaderModuleIdentifierPropertiesEXT(p, "VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT) {
            const VkPhysicalDeviceShaderObjectPropertiesEXT* props = (const VkPhysicalDeviceShaderObjectPropertiesEXT*)structure;
            DumpVkPhysicalDeviceShaderObjectPropertiesEXT(p, "VkPhysicalDeviceShaderObjectPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT) {
            const VkPhysicalDeviceShaderTileImagePropertiesEXT* props = (const VkPhysicalDeviceShaderTileImagePropertiesEXT*)structure;
            DumpVkPhysicalDeviceShaderTileImagePropertiesEXT(p, "VkPhysicalDeviceShaderTileImagePropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES) {
            const VkPhysicalDeviceSubgroupProperties* props = (const VkPhysicalDeviceSubgroupProperties*)structure;
            DumpVkPhysicalDeviceSubgroupProperties(p, "VkPhysicalDeviceSubgroupProperties", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES) {
            const VkPhysicalDeviceSubgroupSizeControlProperties* props = (const VkPhysicalDeviceSubgroupSizeControlProperties*)structure;
            DumpVkPhysicalDeviceSubgroupSizeControlProperties(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceSubgroupSizeControlProperties":"VkPhysicalDeviceSubgroupSizeControlPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES) {
            const VkPhysicalDeviceTexelBufferAlignmentProperties* props = (const VkPhysicalDeviceTexelBufferAlignmentProperties*)structure;
            DumpVkPhysicalDeviceTexelBufferAlignmentProperties(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceTexelBufferAlignmentProperties":"VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES) {
            const VkPhysicalDeviceTimelineSemaphoreProperties* props = (const VkPhysicalDeviceTimelineSemaphoreProperties*)structure;
            DumpVkPhysicalDeviceTimelineSemaphoreProperties(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceTimelineSemaphoreProperties":"VkPhysicalDeviceTimelineSemaphorePropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT) {
            const VkPhysicalDeviceTransformFeedbackPropertiesEXT* props = (const VkPhysicalDeviceTransformFeedbackPropertiesEXT*)structure;
            DumpVkPhysicalDeviceTransformFeedbackPropertiesEXT(p, "VkPhysicalDeviceTransformFeedbackPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES) {
            const VkPhysicalDeviceVertexAttributeDivisorProperties* props = (const VkPhysicalDeviceVertexAttributeDivisorProperties*)structure;
            DumpVkPhysicalDeviceVertexAttributeDivisorProperties(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceVertexAttributeDivisorProperties":"VkPhysicalDeviceVertexAttributeDivisorPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT) {
            const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* props = (const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT*)structure;
            DumpVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(p, "VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES) {
            const VkPhysicalDeviceVulkan11Properties* props = (const VkPhysicalDeviceVulkan11Properties*)structure;
            DumpVkPhysicalDeviceVulkan11Properties(p, "VkPhysicalDeviceVulkan11Properties", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES) {
            const VkPhysicalDeviceVulkan12Properties* props = (const VkPhysicalDeviceVulkan12Properties*)structure;
            DumpVkPhysicalDeviceVulkan12Properties(p, "VkPhysicalDeviceVulkan12Properties", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES) {
            const VkPhysicalDeviceVulkan13Properties* props = (const VkPhysicalDeviceVulkan13Properties*)structure;
            DumpVkPhysicalDeviceVulkan13Properties(p, "VkPhysicalDeviceVulkan13Properties", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES) {
            const VkPhysicalDeviceVulkan14Properties* props = (const VkPhysicalDeviceVulkan14Properties*)structure;
            DumpVkPhysicalDeviceVulkan14Properties(p, "VkPhysicalDeviceVulkan14Properties", *props);
            p.AddNewline();
        }
        place = structure->pNext;
    }
}

bool prepare_phys_device_props2_twocall_chain_vectors(std::unique_ptr<phys_device_props2_chain>& chain) {
    (void)chain;
    chain->VkPhysicalDeviceHostImageCopyProperties_pCopySrcLayouts.resize(chain->PhysicalDeviceHostImageCopyProperties.copySrcLayoutCount);
    chain->PhysicalDeviceHostImageCopyProperties.pCopySrcLayouts = chain->VkPhysicalDeviceHostImageCopyProperties_pCopySrcLayouts.data();
    chain->VkPhysicalDeviceHostImageCopyProperties_pCopyDstLayouts.resize(chain->PhysicalDeviceHostImageCopyProperties.copyDstLayoutCount);
    chain->PhysicalDeviceHostImageCopyProperties.pCopyDstLayouts = chain->VkPhysicalDeviceHostImageCopyProperties_pCopyDstLayouts.data();
    chain->VkPhysicalDeviceLayeredApiPropertiesListKHR_pLayeredApis.resize(chain->PhysicalDeviceLayeredApiPropertiesListKHR.layeredApiCount);
    chain->PhysicalDeviceLayeredApiPropertiesListKHR.pLayeredApis = chain->VkPhysicalDeviceLayeredApiPropertiesListKHR_pLayeredApis.data();
    chain->VkPhysicalDeviceVulkan14Properties_pCopySrcLayouts.resize(chain->PhysicalDeviceVulkan14Properties.copySrcLayoutCount);
    chain->PhysicalDeviceVulkan14Properties.pCopySrcLayouts = chain->VkPhysicalDeviceVulkan14Properties_pCopySrcLayouts.data();
    chain->VkPhysicalDeviceVulkan14Properties_pCopyDstLayouts.resize(chain->PhysicalDeviceVulkan14Properties.copyDstLayoutCount);
    chain->PhysicalDeviceVulkan14Properties.pCopyDstLayouts = chain->VkPhysicalDeviceVulkan14Properties_pCopyDstLayouts.data();
    return true;
}
struct phys_device_mem_props2_chain {
    phys_device_mem_props2_chain() = default;
    phys_device_mem_props2_chain(const phys_device_mem_props2_chain &) = delete;
    phys_device_mem_props2_chain& operator=(const phys_device_mem_props2_chain &) = delete;
    phys_device_mem_props2_chain(phys_device_mem_props2_chain &&) = delete;
    phys_device_mem_props2_chain& operator=(phys_device_mem_props2_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkPhysicalDeviceMemoryBudgetPropertiesEXT PhysicalDeviceMemoryBudgetPropertiesEXT{};
    void initialize_chain(AppGpu &gpu ) noexcept {
        PhysicalDeviceMemoryBudgetPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
        std::vector<VkBaseOutStructure*> chain_members{};
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMemoryBudgetPropertiesEXT));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_phys_device_mem_props2_chain(VkPhysicalDeviceMemoryProperties2& start, std::unique_ptr<phys_device_mem_props2_chain>& chain, AppGpu &gpu){
    chain = std::unique_ptr<phys_device_mem_props2_chain>(new phys_device_mem_props2_chain());
    chain->initialize_chain(gpu);
    start.pNext = chain->start_of_chain;
};

bool prepare_phys_device_mem_props2_twocall_chain_vectors(std::unique_ptr<phys_device_mem_props2_chain>& chain) {
    (void)chain;
    return false;
}
struct phys_device_features2_chain {
    phys_device_features2_chain() = default;
    phys_device_features2_chain(const phys_device_features2_chain &) = delete;
    phys_device_features2_chain& operator=(const phys_device_features2_chain &) = delete;
    phys_device_features2_chain(phys_device_features2_chain &&) = delete;
    phys_device_features2_chain& operator=(phys_device_features2_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkPhysicalDevice16BitStorageFeatures PhysicalDevice16BitStorageFeatures{};
    VkPhysicalDevice4444FormatsFeaturesEXT PhysicalDevice4444FormatsFeaturesEXT{};
    VkPhysicalDevice8BitStorageFeatures PhysicalDevice8BitStorageFeatures{};
    VkPhysicalDeviceASTCDecodeFeaturesEXT PhysicalDeviceASTCDecodeFeaturesEXT{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR PhysicalDeviceAccelerationStructureFeaturesKHR{};
    VkPhysicalDeviceAddressBindingReportFeaturesEXT PhysicalDeviceAddressBindingReportFeaturesEXT{};
    VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT PhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT{};
    VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT PhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT{};
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT PhysicalDeviceBlendOperationAdvancedFeaturesEXT{};
    VkPhysicalDeviceBorderColorSwizzleFeaturesEXT PhysicalDeviceBorderColorSwizzleFeaturesEXT{};
    VkPhysicalDeviceBufferDeviceAddressFeatures PhysicalDeviceBufferDeviceAddressFeatures{};
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT PhysicalDeviceBufferDeviceAddressFeaturesEXT{};
    VkPhysicalDeviceColorWriteEnableFeaturesEXT PhysicalDeviceColorWriteEnableFeaturesEXT{};
    VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR PhysicalDeviceComputeShaderDerivativesFeaturesKHR{};
    VkPhysicalDeviceConditionalRenderingFeaturesEXT PhysicalDeviceConditionalRenderingFeaturesEXT{};
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR PhysicalDeviceCooperativeMatrixFeaturesKHR{};
    VkPhysicalDeviceCustomBorderColorFeaturesEXT PhysicalDeviceCustomBorderColorFeaturesEXT{};
    VkPhysicalDeviceDepthBiasControlFeaturesEXT PhysicalDeviceDepthBiasControlFeaturesEXT{};
    VkPhysicalDeviceDepthClampControlFeaturesEXT PhysicalDeviceDepthClampControlFeaturesEXT{};
    VkPhysicalDeviceDepthClampZeroOneFeaturesKHR PhysicalDeviceDepthClampZeroOneFeaturesKHR{};
    VkPhysicalDeviceDepthClipControlFeaturesEXT PhysicalDeviceDepthClipControlFeaturesEXT{};
    VkPhysicalDeviceDepthClipEnableFeaturesEXT PhysicalDeviceDepthClipEnableFeaturesEXT{};
    VkPhysicalDeviceDescriptorBufferFeaturesEXT PhysicalDeviceDescriptorBufferFeaturesEXT{};
    VkPhysicalDeviceDescriptorIndexingFeatures PhysicalDeviceDescriptorIndexingFeatures{};
    VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT PhysicalDeviceDeviceGeneratedCommandsFeaturesEXT{};
    VkPhysicalDeviceDeviceMemoryReportFeaturesEXT PhysicalDeviceDeviceMemoryReportFeaturesEXT{};
    VkPhysicalDeviceDynamicRenderingFeatures PhysicalDeviceDynamicRenderingFeatures{};
    VkPhysicalDeviceDynamicRenderingLocalReadFeatures PhysicalDeviceDynamicRenderingLocalReadFeatures{};
    VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT{};
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT PhysicalDeviceExtendedDynamicState2FeaturesEXT{};
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT PhysicalDeviceExtendedDynamicState3FeaturesEXT{};
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT PhysicalDeviceExtendedDynamicStateFeaturesEXT{};
    VkPhysicalDeviceFaultFeaturesEXT PhysicalDeviceFaultFeaturesEXT{};
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT PhysicalDeviceFragmentDensityMap2FeaturesEXT{};
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT PhysicalDeviceFragmentDensityMapFeaturesEXT{};
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR PhysicalDeviceFragmentShaderBarycentricFeaturesKHR{};
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT PhysicalDeviceFragmentShaderInterlockFeaturesEXT{};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR PhysicalDeviceFragmentShadingRateFeaturesKHR{};
    VkPhysicalDeviceFrameBoundaryFeaturesEXT PhysicalDeviceFrameBoundaryFeaturesEXT{};
    VkPhysicalDeviceGlobalPriorityQueryFeatures PhysicalDeviceGlobalPriorityQueryFeatures{};
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT{};
    VkPhysicalDeviceHostImageCopyFeatures PhysicalDeviceHostImageCopyFeatures{};
    VkPhysicalDeviceHostQueryResetFeatures PhysicalDeviceHostQueryResetFeatures{};
    VkPhysicalDeviceImage2DViewOf3DFeaturesEXT PhysicalDeviceImage2DViewOf3DFeaturesEXT{};
    VkPhysicalDeviceImageCompressionControlFeaturesEXT PhysicalDeviceImageCompressionControlFeaturesEXT{};
    VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT PhysicalDeviceImageCompressionControlSwapchainFeaturesEXT{};
    VkPhysicalDeviceImageRobustnessFeatures PhysicalDeviceImageRobustnessFeatures{};
    VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT PhysicalDeviceImageSlicedViewOf3DFeaturesEXT{};
    VkPhysicalDeviceImageViewMinLodFeaturesEXT PhysicalDeviceImageViewMinLodFeaturesEXT{};
    VkPhysicalDeviceImagelessFramebufferFeatures PhysicalDeviceImagelessFramebufferFeatures{};
    VkPhysicalDeviceIndexTypeUint8Features PhysicalDeviceIndexTypeUint8Features{};
    VkPhysicalDeviceInlineUniformBlockFeatures PhysicalDeviceInlineUniformBlockFeatures{};
    VkPhysicalDeviceLegacyDitheringFeaturesEXT PhysicalDeviceLegacyDitheringFeaturesEXT{};
    VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT PhysicalDeviceLegacyVertexAttributesFeaturesEXT{};
    VkPhysicalDeviceLineRasterizationFeatures PhysicalDeviceLineRasterizationFeatures{};
    VkPhysicalDeviceMaintenance4Features PhysicalDeviceMaintenance4Features{};
    VkPhysicalDeviceMaintenance5Features PhysicalDeviceMaintenance5Features{};
    VkPhysicalDeviceMaintenance6Features PhysicalDeviceMaintenance6Features{};
    VkPhysicalDeviceMaintenance7FeaturesKHR PhysicalDeviceMaintenance7FeaturesKHR{};
    VkPhysicalDeviceMaintenance8FeaturesKHR PhysicalDeviceMaintenance8FeaturesKHR{};
    VkPhysicalDeviceMapMemoryPlacedFeaturesEXT PhysicalDeviceMapMemoryPlacedFeaturesEXT{};
    VkPhysicalDeviceMemoryPriorityFeaturesEXT PhysicalDeviceMemoryPriorityFeaturesEXT{};
    VkPhysicalDeviceMeshShaderFeaturesEXT PhysicalDeviceMeshShaderFeaturesEXT{};
    VkPhysicalDeviceMultiDrawFeaturesEXT PhysicalDeviceMultiDrawFeaturesEXT{};
    VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT PhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT{};
    VkPhysicalDeviceMultiviewFeatures PhysicalDeviceMultiviewFeatures{};
    VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT PhysicalDeviceMutableDescriptorTypeFeaturesEXT{};
    VkPhysicalDeviceNestedCommandBufferFeaturesEXT PhysicalDeviceNestedCommandBufferFeaturesEXT{};
    VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT PhysicalDeviceNonSeamlessCubeMapFeaturesEXT{};
    VkPhysicalDeviceOpacityMicromapFeaturesEXT PhysicalDeviceOpacityMicromapFeaturesEXT{};
    VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT{};
    VkPhysicalDevicePerformanceQueryFeaturesKHR PhysicalDevicePerformanceQueryFeaturesKHR{};
    VkPhysicalDevicePipelineBinaryFeaturesKHR PhysicalDevicePipelineBinaryFeaturesKHR{};
    VkPhysicalDevicePipelineCreationCacheControlFeatures PhysicalDevicePipelineCreationCacheControlFeatures{};
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR PhysicalDevicePipelineExecutablePropertiesFeaturesKHR{};
    VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT PhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT{};
    VkPhysicalDevicePipelinePropertiesFeaturesEXT PhysicalDevicePipelinePropertiesFeaturesEXT{};
    VkPhysicalDevicePipelineProtectedAccessFeatures PhysicalDevicePipelineProtectedAccessFeatures{};
    VkPhysicalDevicePipelineRobustnessFeatures PhysicalDevicePipelineRobustnessFeatures{};
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VkPhysicalDevicePortabilitySubsetFeaturesKHR PhysicalDevicePortabilitySubsetFeaturesKHR{};
#endif  // VK_ENABLE_BETA_EXTENSIONS
    VkPhysicalDevicePresentIdFeaturesKHR PhysicalDevicePresentIdFeaturesKHR{};
    VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT PhysicalDevicePresentModeFifoLatestReadyFeaturesEXT{};
    VkPhysicalDevicePresentWaitFeaturesKHR PhysicalDevicePresentWaitFeaturesKHR{};
    VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT PhysicalDevicePrimitiveTopologyListRestartFeaturesEXT{};
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT PhysicalDevicePrimitivesGeneratedQueryFeaturesEXT{};
    VkPhysicalDevicePrivateDataFeatures PhysicalDevicePrivateDataFeatures{};
    VkPhysicalDeviceProtectedMemoryFeatures PhysicalDeviceProtectedMemoryFeatures{};
    VkPhysicalDeviceProvokingVertexFeaturesEXT PhysicalDeviceProvokingVertexFeaturesEXT{};
    VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT PhysicalDeviceRGBA10X6FormatsFeaturesEXT{};
    VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT PhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT{};
    VkPhysicalDeviceRayQueryFeaturesKHR PhysicalDeviceRayQueryFeaturesKHR{};
    VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR PhysicalDeviceRayTracingMaintenance1FeaturesKHR{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR PhysicalDeviceRayTracingPipelineFeaturesKHR{};
    VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR PhysicalDeviceRayTracingPositionFetchFeaturesKHR{};
    VkPhysicalDeviceRobustness2FeaturesEXT PhysicalDeviceRobustness2FeaturesEXT{};
    VkPhysicalDeviceSamplerYcbcrConversionFeatures PhysicalDeviceSamplerYcbcrConversionFeatures{};
    VkPhysicalDeviceScalarBlockLayoutFeatures PhysicalDeviceScalarBlockLayoutFeatures{};
    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures PhysicalDeviceSeparateDepthStencilLayoutsFeatures{};
    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT PhysicalDeviceShaderAtomicFloat2FeaturesEXT{};
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT PhysicalDeviceShaderAtomicFloatFeaturesEXT{};
    VkPhysicalDeviceShaderAtomicInt64Features PhysicalDeviceShaderAtomicInt64Features{};
    VkPhysicalDeviceShaderClockFeaturesKHR PhysicalDeviceShaderClockFeaturesKHR{};
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures PhysicalDeviceShaderDemoteToHelperInvocationFeatures{};
    VkPhysicalDeviceShaderDrawParametersFeatures PhysicalDeviceShaderDrawParametersFeatures{};
    VkPhysicalDeviceShaderExpectAssumeFeatures PhysicalDeviceShaderExpectAssumeFeatures{};
    VkPhysicalDeviceShaderFloat16Int8Features PhysicalDeviceShaderFloat16Int8Features{};
    VkPhysicalDeviceShaderFloatControls2Features PhysicalDeviceShaderFloatControls2Features{};
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT PhysicalDeviceShaderImageAtomicInt64FeaturesEXT{};
    VkPhysicalDeviceShaderIntegerDotProductFeatures PhysicalDeviceShaderIntegerDotProductFeatures{};
    char VkPhysicalDeviceShaderIntegerDotProductFeatures_padding[64];
    VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR PhysicalDeviceShaderMaximalReconvergenceFeaturesKHR{};
    VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT PhysicalDeviceShaderModuleIdentifierFeaturesEXT{};
    VkPhysicalDeviceShaderObjectFeaturesEXT PhysicalDeviceShaderObjectFeaturesEXT{};
    VkPhysicalDeviceShaderQuadControlFeaturesKHR PhysicalDeviceShaderQuadControlFeaturesKHR{};
    VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR PhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR{};
    VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT PhysicalDeviceShaderReplicatedCompositesFeaturesEXT{};
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures PhysicalDeviceShaderSubgroupExtendedTypesFeatures{};
    VkPhysicalDeviceShaderSubgroupRotateFeatures PhysicalDeviceShaderSubgroupRotateFeatures{};
    VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR PhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR{};
    VkPhysicalDeviceShaderTerminateInvocationFeatures PhysicalDeviceShaderTerminateInvocationFeatures{};
    VkPhysicalDeviceShaderTileImageFeaturesEXT PhysicalDeviceShaderTileImageFeaturesEXT{};
    VkPhysicalDeviceSubgroupSizeControlFeatures PhysicalDeviceSubgroupSizeControlFeatures{};
    VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT PhysicalDeviceSubpassMergeFeedbackFeaturesEXT{};
    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT PhysicalDeviceSwapchainMaintenance1FeaturesEXT{};
    VkPhysicalDeviceSynchronization2Features PhysicalDeviceSynchronization2Features{};
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT PhysicalDeviceTexelBufferAlignmentFeaturesEXT{};
    VkPhysicalDeviceTextureCompressionASTCHDRFeatures PhysicalDeviceTextureCompressionASTCHDRFeatures{};
    VkPhysicalDeviceTimelineSemaphoreFeatures PhysicalDeviceTimelineSemaphoreFeatures{};
    VkPhysicalDeviceTransformFeedbackFeaturesEXT PhysicalDeviceTransformFeedbackFeaturesEXT{};
    VkPhysicalDeviceUniformBufferStandardLayoutFeatures PhysicalDeviceUniformBufferStandardLayoutFeatures{};
    VkPhysicalDeviceVariablePointersFeatures PhysicalDeviceVariablePointersFeatures{};
    VkPhysicalDeviceVertexAttributeDivisorFeatures PhysicalDeviceVertexAttributeDivisorFeatures{};
    VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT PhysicalDeviceVertexAttributeRobustnessFeaturesEXT{};
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT PhysicalDeviceVertexInputDynamicStateFeaturesEXT{};
    VkPhysicalDeviceVideoEncodeAV1FeaturesKHR PhysicalDeviceVideoEncodeAV1FeaturesKHR{};
    VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR PhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR{};
    VkPhysicalDeviceVideoMaintenance1FeaturesKHR PhysicalDeviceVideoMaintenance1FeaturesKHR{};
    VkPhysicalDeviceVideoMaintenance2FeaturesKHR PhysicalDeviceVideoMaintenance2FeaturesKHR{};
    VkPhysicalDeviceVulkan11Features PhysicalDeviceVulkan11Features{};
    VkPhysicalDeviceVulkan12Features PhysicalDeviceVulkan12Features{};
    VkPhysicalDeviceVulkan13Features PhysicalDeviceVulkan13Features{};
    VkPhysicalDeviceVulkan14Features PhysicalDeviceVulkan14Features{};
    VkPhysicalDeviceVulkanMemoryModelFeatures PhysicalDeviceVulkanMemoryModelFeatures{};
    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR PhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR{};
    VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT PhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT{};
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT PhysicalDeviceYcbcrImageArraysFeaturesEXT{};
    VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures PhysicalDeviceZeroInitializeWorkgroupMemoryFeatures{};
    void initialize_chain(AppGpu &gpu , bool show_promoted_structs) noexcept {
        PhysicalDevice16BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
        PhysicalDevice4444FormatsFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT;
        PhysicalDevice8BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;
        PhysicalDeviceASTCDecodeFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT;
        PhysicalDeviceAccelerationStructureFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        PhysicalDeviceAddressBindingReportFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT;
        PhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT;
        PhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT;
        PhysicalDeviceBlendOperationAdvancedFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;
        PhysicalDeviceBorderColorSwizzleFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT;
        PhysicalDeviceBufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        PhysicalDeviceBufferDeviceAddressFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
        PhysicalDeviceColorWriteEnableFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT;
        PhysicalDeviceComputeShaderDerivativesFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR;
        PhysicalDeviceConditionalRenderingFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT;
        PhysicalDeviceCooperativeMatrixFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR;
        PhysicalDeviceCustomBorderColorFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
        PhysicalDeviceDepthBiasControlFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT;
        PhysicalDeviceDepthClampControlFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT;
        PhysicalDeviceDepthClampZeroOneFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR;
        PhysicalDeviceDepthClipControlFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT;
        PhysicalDeviceDepthClipEnableFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
        PhysicalDeviceDescriptorBufferFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
        PhysicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        PhysicalDeviceDeviceGeneratedCommandsFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT;
        PhysicalDeviceDeviceMemoryReportFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT;
        PhysicalDeviceDynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        PhysicalDeviceDynamicRenderingLocalReadFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES;
        PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT;
        PhysicalDeviceExtendedDynamicState2FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
        PhysicalDeviceExtendedDynamicState3FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
        PhysicalDeviceExtendedDynamicStateFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
        PhysicalDeviceFaultFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT;
        PhysicalDeviceFragmentDensityMap2FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT;
        PhysicalDeviceFragmentDensityMapFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT;
        PhysicalDeviceFragmentShaderBarycentricFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR;
        PhysicalDeviceFragmentShaderInterlockFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT;
        PhysicalDeviceFragmentShadingRateFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
        PhysicalDeviceFrameBoundaryFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT;
        PhysicalDeviceGlobalPriorityQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES;
        PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT;
        PhysicalDeviceHostImageCopyFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES;
        PhysicalDeviceHostQueryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
        PhysicalDeviceImage2DViewOf3DFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT;
        PhysicalDeviceImageCompressionControlFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT;
        PhysicalDeviceImageCompressionControlSwapchainFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT;
        PhysicalDeviceImageRobustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES;
        PhysicalDeviceImageSlicedViewOf3DFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT;
        PhysicalDeviceImageViewMinLodFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT;
        PhysicalDeviceImagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
        PhysicalDeviceIndexTypeUint8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES;
        PhysicalDeviceInlineUniformBlockFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES;
        PhysicalDeviceLegacyDitheringFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT;
        PhysicalDeviceLegacyVertexAttributesFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT;
        PhysicalDeviceLineRasterizationFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES;
        PhysicalDeviceMaintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
        PhysicalDeviceMaintenance5Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES;
        PhysicalDeviceMaintenance6Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES;
        PhysicalDeviceMaintenance7FeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR;
        PhysicalDeviceMaintenance8FeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR;
        PhysicalDeviceMapMemoryPlacedFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT;
        PhysicalDeviceMemoryPriorityFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT;
        PhysicalDeviceMeshShaderFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
        PhysicalDeviceMultiDrawFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT;
        PhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT;
        PhysicalDeviceMultiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
        PhysicalDeviceMutableDescriptorTypeFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT;
        PhysicalDeviceNestedCommandBufferFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT;
        PhysicalDeviceNonSeamlessCubeMapFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT;
        PhysicalDeviceOpacityMicromapFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT;
        PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT;
        PhysicalDevicePerformanceQueryFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR;
        PhysicalDevicePipelineBinaryFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR;
        PhysicalDevicePipelineCreationCacheControlFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
        PhysicalDevicePipelineExecutablePropertiesFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
        PhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT;
        PhysicalDevicePipelinePropertiesFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT;
        PhysicalDevicePipelineProtectedAccessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES;
        PhysicalDevicePipelineRobustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        PhysicalDevicePortabilitySubsetFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        PhysicalDevicePresentIdFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR;
        PhysicalDevicePresentModeFifoLatestReadyFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT;
        PhysicalDevicePresentWaitFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR;
        PhysicalDevicePrimitiveTopologyListRestartFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
        PhysicalDevicePrimitivesGeneratedQueryFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT;
        PhysicalDevicePrivateDataFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES;
        PhysicalDeviceProtectedMemoryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;
        PhysicalDeviceProvokingVertexFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT;
        PhysicalDeviceRGBA10X6FormatsFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT;
        PhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT;
        PhysicalDeviceRayQueryFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
        PhysicalDeviceRayTracingMaintenance1FeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR;
        PhysicalDeviceRayTracingPipelineFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        PhysicalDeviceRayTracingPositionFetchFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR;
        PhysicalDeviceRobustness2FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
        PhysicalDeviceSamplerYcbcrConversionFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
        PhysicalDeviceScalarBlockLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
        PhysicalDeviceSeparateDepthStencilLayoutsFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES;
        PhysicalDeviceShaderAtomicFloat2FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT;
        PhysicalDeviceShaderAtomicFloatFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
        PhysicalDeviceShaderAtomicInt64Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES;
        PhysicalDeviceShaderClockFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR;
        PhysicalDeviceShaderDemoteToHelperInvocationFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES;
        PhysicalDeviceShaderDrawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
        PhysicalDeviceShaderExpectAssumeFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES;
        PhysicalDeviceShaderFloat16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
        PhysicalDeviceShaderFloatControls2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES;
        PhysicalDeviceShaderImageAtomicInt64FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT;
        PhysicalDeviceShaderIntegerDotProductFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES;
        PhysicalDeviceShaderMaximalReconvergenceFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR;
        PhysicalDeviceShaderModuleIdentifierFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT;
        PhysicalDeviceShaderObjectFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
        PhysicalDeviceShaderQuadControlFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR;
        PhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR;
        PhysicalDeviceShaderReplicatedCompositesFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT;
        PhysicalDeviceShaderSubgroupExtendedTypesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES;
        PhysicalDeviceShaderSubgroupRotateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES;
        PhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR;
        PhysicalDeviceShaderTerminateInvocationFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES;
        PhysicalDeviceShaderTileImageFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT;
        PhysicalDeviceSubgroupSizeControlFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES;
        PhysicalDeviceSubpassMergeFeedbackFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT;
        PhysicalDeviceSwapchainMaintenance1FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT;
        PhysicalDeviceSynchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        PhysicalDeviceTexelBufferAlignmentFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT;
        PhysicalDeviceTextureCompressionASTCHDRFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES;
        PhysicalDeviceTimelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        PhysicalDeviceTransformFeedbackFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT;
        PhysicalDeviceUniformBufferStandardLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;
        PhysicalDeviceVariablePointersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES;
        PhysicalDeviceVertexAttributeDivisorFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES;
        PhysicalDeviceVertexAttributeRobustnessFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT;
        PhysicalDeviceVertexInputDynamicStateFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
        PhysicalDeviceVideoEncodeAV1FeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR;
        PhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR;
        PhysicalDeviceVideoMaintenance1FeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR;
        PhysicalDeviceVideoMaintenance2FeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR;
        PhysicalDeviceVulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        PhysicalDeviceVulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        PhysicalDeviceVulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        PhysicalDeviceVulkan14Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
        PhysicalDeviceVulkanMemoryModelFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES;
        PhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR;
        PhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT;
        PhysicalDeviceYcbcrImageArraysFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT;
        PhysicalDeviceZeroInitializeWorkgroupMemoryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES;
        std::vector<VkBaseOutStructure*> chain_members{};
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_16BIT_STORAGE_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_1 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevice16BitStorageFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_4444_FORMATS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevice4444FormatsFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_8BIT_STORAGE_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevice8BitStorageFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceASTCDecodeFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceAccelerationStructureFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceAddressBindingReportFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceBlendOperationAdvancedFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_BORDER_COLOR_SWIZZLE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceBorderColorSwizzleFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceBufferDeviceAddressFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceBufferDeviceAddressFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceColorWriteEnableFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceComputeShaderDerivativesFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceConditionalRenderingFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceCooperativeMatrixFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceCustomBorderColorFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDepthBiasControlFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEPTH_CLAMP_CONTROL_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDepthClampControlFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEPTH_CLAMP_ZERO_ONE_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_DEPTH_CLAMP_ZERO_ONE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDepthClampZeroOneFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDepthClipControlFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDepthClipEnableFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDescriptorBufferFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDescriptorIndexingFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDeviceGeneratedCommandsFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDeviceMemoryReportFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDynamicRenderingFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDynamicRenderingLocalReadFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceExtendedDynamicState2FeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceExtendedDynamicState3FeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceExtendedDynamicStateFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_DEVICE_FAULT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFaultFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentDensityMap2FeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentDensityMapFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentShaderBarycentricFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentShaderInterlockFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFragmentShadingRateFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_FRAME_BOUNDARY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceFrameBoundaryFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_GLOBAL_PRIORITY_QUERY_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceGlobalPriorityQueryFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceHostImageCopyFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceHostQueryResetFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceImage2DViewOf3DFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceImageCompressionControlFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceImageCompressionControlSwapchainFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceImageRobustnessFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceImageSlicedViewOf3DFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_IMAGE_VIEW_MIN_LOD_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceImageViewMinLodFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceImagelessFramebufferFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceIndexTypeUint8Features));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceInlineUniformBlockFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_LEGACY_DITHERING_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceLegacyDitheringFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_LEGACY_VERTEX_ATTRIBUTES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceLegacyVertexAttributesFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceLineRasterizationFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_4_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance4Features));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_5_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance5Features));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_6_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance6Features));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_7_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance7FeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MAINTENANCE_8_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMaintenance8FeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MAP_MEMORY_PLACED_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMapMemoryPlacedFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMemoryPriorityFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MESH_SHADER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMeshShaderFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MULTI_DRAW_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMultiDrawFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_MULTIVIEW_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_1 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMultiviewFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceMutableDescriptorTypeFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceNestedCommandBufferFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_NON_SEAMLESS_CUBE_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceNonSeamlessCubeMapFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceOpacityMicromapFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePerformanceQueryFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelineBinaryFeaturesKHR));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelineCreationCacheControlFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelineExecutablePropertiesFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PIPELINE_LIBRARY_GROUP_HANDLES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PIPELINE_PROPERTIES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelinePropertiesFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelineProtectedAccessFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePipelineRobustnessFeatures));
#ifdef VK_ENABLE_BETA_EXTENSIONS
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePortabilitySubsetFeaturesKHR));
#endif  // VK_ENABLE_BETA_EXTENSIONS
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PRESENT_ID_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePresentIdFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePresentModeFifoLatestReadyFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_PRESENT_WAIT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePresentWaitFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePrimitiveTopologyListRestartFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePrimitivesGeneratedQueryFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PRIVATE_DATA_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDevicePrivateDataFeatures));
        if (gpu.api_version == VK_API_VERSION_1_1 || show_promoted_structs)
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceProtectedMemoryFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceProvokingVertexFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_RGBA10X6_FORMATS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRGBA10X6FormatsFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_RAY_QUERY_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRayQueryFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRayTracingMaintenance1FeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRayTracingPipelineFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRayTracingPositionFetchFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceRobustness2FeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_1 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSamplerYcbcrConversionFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceScalarBlockLayoutFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSeparateDepthStencilLayoutsFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderAtomicFloat2FeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderAtomicFloatFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderAtomicInt64Features));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_CLOCK_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderClockFeaturesKHR));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderDemoteToHelperInvocationFeatures));
        if (gpu.api_version == VK_API_VERSION_1_1 || show_promoted_structs)
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderDrawParametersFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_EXPECT_ASSUME_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderExpectAssumeFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderFloat16Int8Features));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_FLOAT_CONTROLS_2_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderFloatControls2Features));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderImageAtomicInt64FeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderIntegerDotProductFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_MAXIMAL_RECONVERGENCE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderMaximalReconvergenceFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderModuleIdentifierFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_OBJECT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderObjectFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_QUAD_CONTROL_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderQuadControlFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_RELAXED_EXTENDED_INSTRUCTION_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_REPLICATED_COMPOSITES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderReplicatedCompositesFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderSubgroupExtendedTypesFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_SUBGROUP_ROTATE_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderSubgroupRotateFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderTerminateInvocationFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceShaderTileImageFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSubgroupSizeControlFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SUBPASS_MERGE_FEEDBACK_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSubpassMergeFeedbackFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSwapchainMaintenance1FeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceSynchronization2Features));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceTexelBufferAlignmentFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceTextureCompressionASTCHDRFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceTimelineSemaphoreFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceTransformFeedbackFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceUniformBufferStandardLayoutFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_1 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVariablePointersFeatures));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVertexAttributeDivisorFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_VERTEX_ATTRIBUTE_ROBUSTNESS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVertexAttributeRobustnessFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVertexInputDynamicStateFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_AV1_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVideoEncodeAV1FeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVideoMaintenance1FeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVideoMaintenance2FeaturesKHR));
        if ((gpu.api_version >= VK_API_VERSION_1_2))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkan11Features));
        if ((gpu.api_version >= VK_API_VERSION_1_2))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkan12Features));
        if ((gpu.api_version >= VK_API_VERSION_1_3))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkan13Features));
        if ((gpu.api_version >= VK_API_VERSION_1_4))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkan14Features));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_2 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceVulkanMemoryModelFeatures));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceYcbcrImageArraysFeaturesEXT));
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 || show_promoted_structs))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&PhysicalDeviceZeroInitializeWorkgroupMemoryFeatures));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_phys_device_features2_chain(VkPhysicalDeviceFeatures2& start, std::unique_ptr<phys_device_features2_chain>& chain, AppGpu &gpu,bool show_promoted_structs){
    chain = std::unique_ptr<phys_device_features2_chain>(new phys_device_features2_chain());
    chain->initialize_chain(gpu,show_promoted_structs);
    start.pNext = chain->start_of_chain;
};

void chain_iterator_phys_device_features2(Printer &p, AppGpu &gpu, bool show_promoted_structs, const void * place) {
    while (place) {
        const VkBaseOutStructure *structure = (const VkBaseOutStructure *)place;
        p.SetSubHeader();
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES) {
            const VkPhysicalDevice16BitStorageFeatures* props = (const VkPhysicalDevice16BitStorageFeatures*)structure;
            DumpVkPhysicalDevice16BitStorageFeatures(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDevice16BitStorageFeatures":"VkPhysicalDevice16BitStorageFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT) {
            const VkPhysicalDevice4444FormatsFeaturesEXT* props = (const VkPhysicalDevice4444FormatsFeaturesEXT*)structure;
            DumpVkPhysicalDevice4444FormatsFeaturesEXT(p, "VkPhysicalDevice4444FormatsFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES) {
            const VkPhysicalDevice8BitStorageFeatures* props = (const VkPhysicalDevice8BitStorageFeatures*)structure;
            DumpVkPhysicalDevice8BitStorageFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDevice8BitStorageFeatures":"VkPhysicalDevice8BitStorageFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT) {
            const VkPhysicalDeviceASTCDecodeFeaturesEXT* props = (const VkPhysicalDeviceASTCDecodeFeaturesEXT*)structure;
            DumpVkPhysicalDeviceASTCDecodeFeaturesEXT(p, "VkPhysicalDeviceASTCDecodeFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR) {
            const VkPhysicalDeviceAccelerationStructureFeaturesKHR* props = (const VkPhysicalDeviceAccelerationStructureFeaturesKHR*)structure;
            DumpVkPhysicalDeviceAccelerationStructureFeaturesKHR(p, "VkPhysicalDeviceAccelerationStructureFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT) {
            const VkPhysicalDeviceAddressBindingReportFeaturesEXT* props = (const VkPhysicalDeviceAddressBindingReportFeaturesEXT*)structure;
            DumpVkPhysicalDeviceAddressBindingReportFeaturesEXT(p, "VkPhysicalDeviceAddressBindingReportFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT) {
            const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT* props = (const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT*)structure;
            DumpVkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT(p, "VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT) {
            const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT* props = (const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT*)structure;
            DumpVkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(p, "VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT) {
            const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* props = (const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT*)structure;
            DumpVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(p, "VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT) {
            const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT* props = (const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT*)structure;
            DumpVkPhysicalDeviceBorderColorSwizzleFeaturesEXT(p, "VkPhysicalDeviceBorderColorSwizzleFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES) {
            const VkPhysicalDeviceBufferDeviceAddressFeatures* props = (const VkPhysicalDeviceBufferDeviceAddressFeatures*)structure;
            DumpVkPhysicalDeviceBufferDeviceAddressFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceBufferDeviceAddressFeatures":"VkPhysicalDeviceBufferDeviceAddressFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT) {
            const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* props = (const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT*)structure;
            DumpVkPhysicalDeviceBufferDeviceAddressFeaturesEXT(p, "VkPhysicalDeviceBufferDeviceAddressFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT) {
            const VkPhysicalDeviceColorWriteEnableFeaturesEXT* props = (const VkPhysicalDeviceColorWriteEnableFeaturesEXT*)structure;
            DumpVkPhysicalDeviceColorWriteEnableFeaturesEXT(p, "VkPhysicalDeviceColorWriteEnableFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR) {
            const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR* props = (const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR*)structure;
            DumpVkPhysicalDeviceComputeShaderDerivativesFeaturesKHR(p, "VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT) {
            const VkPhysicalDeviceConditionalRenderingFeaturesEXT* props = (const VkPhysicalDeviceConditionalRenderingFeaturesEXT*)structure;
            DumpVkPhysicalDeviceConditionalRenderingFeaturesEXT(p, "VkPhysicalDeviceConditionalRenderingFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR) {
            const VkPhysicalDeviceCooperativeMatrixFeaturesKHR* props = (const VkPhysicalDeviceCooperativeMatrixFeaturesKHR*)structure;
            DumpVkPhysicalDeviceCooperativeMatrixFeaturesKHR(p, "VkPhysicalDeviceCooperativeMatrixFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT) {
            const VkPhysicalDeviceCustomBorderColorFeaturesEXT* props = (const VkPhysicalDeviceCustomBorderColorFeaturesEXT*)structure;
            DumpVkPhysicalDeviceCustomBorderColorFeaturesEXT(p, "VkPhysicalDeviceCustomBorderColorFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT) {
            const VkPhysicalDeviceDepthBiasControlFeaturesEXT* props = (const VkPhysicalDeviceDepthBiasControlFeaturesEXT*)structure;
            DumpVkPhysicalDeviceDepthBiasControlFeaturesEXT(p, "VkPhysicalDeviceDepthBiasControlFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT) {
            const VkPhysicalDeviceDepthClampControlFeaturesEXT* props = (const VkPhysicalDeviceDepthClampControlFeaturesEXT*)structure;
            DumpVkPhysicalDeviceDepthClampControlFeaturesEXT(p, "VkPhysicalDeviceDepthClampControlFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR) {
            const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR* props = (const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR*)structure;
            DumpVkPhysicalDeviceDepthClampZeroOneFeaturesKHR(p, "VkPhysicalDeviceDepthClampZeroOneFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT) {
            const VkPhysicalDeviceDepthClipControlFeaturesEXT* props = (const VkPhysicalDeviceDepthClipControlFeaturesEXT*)structure;
            DumpVkPhysicalDeviceDepthClipControlFeaturesEXT(p, "VkPhysicalDeviceDepthClipControlFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT) {
            const VkPhysicalDeviceDepthClipEnableFeaturesEXT* props = (const VkPhysicalDeviceDepthClipEnableFeaturesEXT*)structure;
            DumpVkPhysicalDeviceDepthClipEnableFeaturesEXT(p, "VkPhysicalDeviceDepthClipEnableFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT) {
            const VkPhysicalDeviceDescriptorBufferFeaturesEXT* props = (const VkPhysicalDeviceDescriptorBufferFeaturesEXT*)structure;
            DumpVkPhysicalDeviceDescriptorBufferFeaturesEXT(p, "VkPhysicalDeviceDescriptorBufferFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES) {
            const VkPhysicalDeviceDescriptorIndexingFeatures* props = (const VkPhysicalDeviceDescriptorIndexingFeatures*)structure;
            DumpVkPhysicalDeviceDescriptorIndexingFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceDescriptorIndexingFeatures":"VkPhysicalDeviceDescriptorIndexingFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT) {
            const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT* props = (const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT*)structure;
            DumpVkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT(p, "VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT) {
            const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT* props = (const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT*)structure;
            DumpVkPhysicalDeviceDeviceMemoryReportFeaturesEXT(p, "VkPhysicalDeviceDeviceMemoryReportFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES) {
            const VkPhysicalDeviceDynamicRenderingFeatures* props = (const VkPhysicalDeviceDynamicRenderingFeatures*)structure;
            DumpVkPhysicalDeviceDynamicRenderingFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceDynamicRenderingFeatures":"VkPhysicalDeviceDynamicRenderingFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES) {
            const VkPhysicalDeviceDynamicRenderingLocalReadFeatures* props = (const VkPhysicalDeviceDynamicRenderingLocalReadFeatures*)structure;
            DumpVkPhysicalDeviceDynamicRenderingLocalReadFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceDynamicRenderingLocalReadFeatures":"VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT) {
            const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT* props = (const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT*)structure;
            DumpVkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT(p, "VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT) {
            const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* props = (const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT*)structure;
            DumpVkPhysicalDeviceExtendedDynamicState2FeaturesEXT(p, "VkPhysicalDeviceExtendedDynamicState2FeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT) {
            const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT* props = (const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT*)structure;
            DumpVkPhysicalDeviceExtendedDynamicState3FeaturesEXT(p, "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT) {
            const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* props = (const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*)structure;
            DumpVkPhysicalDeviceExtendedDynamicStateFeaturesEXT(p, "VkPhysicalDeviceExtendedDynamicStateFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT) {
            const VkPhysicalDeviceFaultFeaturesEXT* props = (const VkPhysicalDeviceFaultFeaturesEXT*)structure;
            DumpVkPhysicalDeviceFaultFeaturesEXT(p, "VkPhysicalDeviceFaultFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT) {
            const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT* props = (const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT*)structure;
            DumpVkPhysicalDeviceFragmentDensityMap2FeaturesEXT(p, "VkPhysicalDeviceFragmentDensityMap2FeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT) {
            const VkPhysicalDeviceFragmentDensityMapFeaturesEXT* props = (const VkPhysicalDeviceFragmentDensityMapFeaturesEXT*)structure;
            DumpVkPhysicalDeviceFragmentDensityMapFeaturesEXT(p, "VkPhysicalDeviceFragmentDensityMapFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR) {
            const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* props = (const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR*)structure;
            DumpVkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(p, "VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT) {
            const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* props = (const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT*)structure;
            DumpVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(p, "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR) {
            const VkPhysicalDeviceFragmentShadingRateFeaturesKHR* props = (const VkPhysicalDeviceFragmentShadingRateFeaturesKHR*)structure;
            DumpVkPhysicalDeviceFragmentShadingRateFeaturesKHR(p, "VkPhysicalDeviceFragmentShadingRateFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT) {
            const VkPhysicalDeviceFrameBoundaryFeaturesEXT* props = (const VkPhysicalDeviceFrameBoundaryFeaturesEXT*)structure;
            DumpVkPhysicalDeviceFrameBoundaryFeaturesEXT(p, "VkPhysicalDeviceFrameBoundaryFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES) {
            const VkPhysicalDeviceGlobalPriorityQueryFeatures* props = (const VkPhysicalDeviceGlobalPriorityQueryFeatures*)structure;
            DumpVkPhysicalDeviceGlobalPriorityQueryFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceGlobalPriorityQueryFeatures":"VkPhysicalDeviceGlobalPriorityQueryFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT) {
            const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT* props = (const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT*)structure;
            DumpVkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(p, "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES) {
            const VkPhysicalDeviceHostImageCopyFeatures* props = (const VkPhysicalDeviceHostImageCopyFeatures*)structure;
            DumpVkPhysicalDeviceHostImageCopyFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceHostImageCopyFeatures":"VkPhysicalDeviceHostImageCopyFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES) {
            const VkPhysicalDeviceHostQueryResetFeatures* props = (const VkPhysicalDeviceHostQueryResetFeatures*)structure;
            DumpVkPhysicalDeviceHostQueryResetFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceHostQueryResetFeatures":"VkPhysicalDeviceHostQueryResetFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT) {
            const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT* props = (const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT*)structure;
            DumpVkPhysicalDeviceImage2DViewOf3DFeaturesEXT(p, "VkPhysicalDeviceImage2DViewOf3DFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT) {
            const VkPhysicalDeviceImageCompressionControlFeaturesEXT* props = (const VkPhysicalDeviceImageCompressionControlFeaturesEXT*)structure;
            DumpVkPhysicalDeviceImageCompressionControlFeaturesEXT(p, "VkPhysicalDeviceImageCompressionControlFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT) {
            const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT* props = (const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT*)structure;
            DumpVkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(p, "VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES) {
            const VkPhysicalDeviceImageRobustnessFeatures* props = (const VkPhysicalDeviceImageRobustnessFeatures*)structure;
            DumpVkPhysicalDeviceImageRobustnessFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceImageRobustnessFeatures":"VkPhysicalDeviceImageRobustnessFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT) {
            const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT* props = (const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT*)structure;
            DumpVkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT(p, "VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT) {
            const VkPhysicalDeviceImageViewMinLodFeaturesEXT* props = (const VkPhysicalDeviceImageViewMinLodFeaturesEXT*)structure;
            DumpVkPhysicalDeviceImageViewMinLodFeaturesEXT(p, "VkPhysicalDeviceImageViewMinLodFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES) {
            const VkPhysicalDeviceImagelessFramebufferFeatures* props = (const VkPhysicalDeviceImagelessFramebufferFeatures*)structure;
            DumpVkPhysicalDeviceImagelessFramebufferFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceImagelessFramebufferFeatures":"VkPhysicalDeviceImagelessFramebufferFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES) {
            const VkPhysicalDeviceIndexTypeUint8Features* props = (const VkPhysicalDeviceIndexTypeUint8Features*)structure;
            DumpVkPhysicalDeviceIndexTypeUint8Features(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceIndexTypeUint8Features":"VkPhysicalDeviceIndexTypeUint8FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES) {
            const VkPhysicalDeviceInlineUniformBlockFeatures* props = (const VkPhysicalDeviceInlineUniformBlockFeatures*)structure;
            DumpVkPhysicalDeviceInlineUniformBlockFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceInlineUniformBlockFeatures":"VkPhysicalDeviceInlineUniformBlockFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT) {
            const VkPhysicalDeviceLegacyDitheringFeaturesEXT* props = (const VkPhysicalDeviceLegacyDitheringFeaturesEXT*)structure;
            DumpVkPhysicalDeviceLegacyDitheringFeaturesEXT(p, "VkPhysicalDeviceLegacyDitheringFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT) {
            const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT* props = (const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT*)structure;
            DumpVkPhysicalDeviceLegacyVertexAttributesFeaturesEXT(p, "VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES) {
            const VkPhysicalDeviceLineRasterizationFeatures* props = (const VkPhysicalDeviceLineRasterizationFeatures*)structure;
            DumpVkPhysicalDeviceLineRasterizationFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceLineRasterizationFeatures":"VkPhysicalDeviceLineRasterizationFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES) {
            const VkPhysicalDeviceMaintenance4Features* props = (const VkPhysicalDeviceMaintenance4Features*)structure;
            DumpVkPhysicalDeviceMaintenance4Features(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceMaintenance4Features":"VkPhysicalDeviceMaintenance4FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES) {
            const VkPhysicalDeviceMaintenance5Features* props = (const VkPhysicalDeviceMaintenance5Features*)structure;
            DumpVkPhysicalDeviceMaintenance5Features(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceMaintenance5Features":"VkPhysicalDeviceMaintenance5FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES) {
            const VkPhysicalDeviceMaintenance6Features* props = (const VkPhysicalDeviceMaintenance6Features*)structure;
            DumpVkPhysicalDeviceMaintenance6Features(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceMaintenance6Features":"VkPhysicalDeviceMaintenance6FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR) {
            const VkPhysicalDeviceMaintenance7FeaturesKHR* props = (const VkPhysicalDeviceMaintenance7FeaturesKHR*)structure;
            DumpVkPhysicalDeviceMaintenance7FeaturesKHR(p, "VkPhysicalDeviceMaintenance7FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR) {
            const VkPhysicalDeviceMaintenance8FeaturesKHR* props = (const VkPhysicalDeviceMaintenance8FeaturesKHR*)structure;
            DumpVkPhysicalDeviceMaintenance8FeaturesKHR(p, "VkPhysicalDeviceMaintenance8FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT) {
            const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT* props = (const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT*)structure;
            DumpVkPhysicalDeviceMapMemoryPlacedFeaturesEXT(p, "VkPhysicalDeviceMapMemoryPlacedFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT) {
            const VkPhysicalDeviceMemoryPriorityFeaturesEXT* props = (const VkPhysicalDeviceMemoryPriorityFeaturesEXT*)structure;
            DumpVkPhysicalDeviceMemoryPriorityFeaturesEXT(p, "VkPhysicalDeviceMemoryPriorityFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT) {
            const VkPhysicalDeviceMeshShaderFeaturesEXT* props = (const VkPhysicalDeviceMeshShaderFeaturesEXT*)structure;
            DumpVkPhysicalDeviceMeshShaderFeaturesEXT(p, "VkPhysicalDeviceMeshShaderFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT) {
            const VkPhysicalDeviceMultiDrawFeaturesEXT* props = (const VkPhysicalDeviceMultiDrawFeaturesEXT*)structure;
            DumpVkPhysicalDeviceMultiDrawFeaturesEXT(p, "VkPhysicalDeviceMultiDrawFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT) {
            const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT* props = (const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT*)structure;
            DumpVkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(p, "VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES) {
            const VkPhysicalDeviceMultiviewFeatures* props = (const VkPhysicalDeviceMultiviewFeatures*)structure;
            DumpVkPhysicalDeviceMultiviewFeatures(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDeviceMultiviewFeatures":"VkPhysicalDeviceMultiviewFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT) {
            const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT* props = (const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT*)structure;
            DumpVkPhysicalDeviceMutableDescriptorTypeFeaturesEXT(p, "VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT) {
            const VkPhysicalDeviceNestedCommandBufferFeaturesEXT* props = (const VkPhysicalDeviceNestedCommandBufferFeaturesEXT*)structure;
            DumpVkPhysicalDeviceNestedCommandBufferFeaturesEXT(p, "VkPhysicalDeviceNestedCommandBufferFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT) {
            const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT* props = (const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT*)structure;
            DumpVkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT(p, "VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT) {
            const VkPhysicalDeviceOpacityMicromapFeaturesEXT* props = (const VkPhysicalDeviceOpacityMicromapFeaturesEXT*)structure;
            DumpVkPhysicalDeviceOpacityMicromapFeaturesEXT(p, "VkPhysicalDeviceOpacityMicromapFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT) {
            const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT* props = (const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT*)structure;
            DumpVkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(p, "VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR) {
            const VkPhysicalDevicePerformanceQueryFeaturesKHR* props = (const VkPhysicalDevicePerformanceQueryFeaturesKHR*)structure;
            DumpVkPhysicalDevicePerformanceQueryFeaturesKHR(p, "VkPhysicalDevicePerformanceQueryFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR) {
            const VkPhysicalDevicePipelineBinaryFeaturesKHR* props = (const VkPhysicalDevicePipelineBinaryFeaturesKHR*)structure;
            DumpVkPhysicalDevicePipelineBinaryFeaturesKHR(p, "VkPhysicalDevicePipelineBinaryFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES) {
            const VkPhysicalDevicePipelineCreationCacheControlFeatures* props = (const VkPhysicalDevicePipelineCreationCacheControlFeatures*)structure;
            DumpVkPhysicalDevicePipelineCreationCacheControlFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDevicePipelineCreationCacheControlFeatures":"VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR) {
            const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* props = (const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR*)structure;
            DumpVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(p, "VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT) {
            const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT* props = (const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT*)structure;
            DumpVkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(p, "VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT) {
            const VkPhysicalDevicePipelinePropertiesFeaturesEXT* props = (const VkPhysicalDevicePipelinePropertiesFeaturesEXT*)structure;
            DumpVkPhysicalDevicePipelinePropertiesFeaturesEXT(p, "VkPhysicalDevicePipelinePropertiesFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES) {
            const VkPhysicalDevicePipelineProtectedAccessFeatures* props = (const VkPhysicalDevicePipelineProtectedAccessFeatures*)structure;
            DumpVkPhysicalDevicePipelineProtectedAccessFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDevicePipelineProtectedAccessFeatures":"VkPhysicalDevicePipelineProtectedAccessFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES) {
            const VkPhysicalDevicePipelineRobustnessFeatures* props = (const VkPhysicalDevicePipelineRobustnessFeatures*)structure;
            DumpVkPhysicalDevicePipelineRobustnessFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDevicePipelineRobustnessFeatures":"VkPhysicalDevicePipelineRobustnessFeaturesEXT", *props);
            p.AddNewline();
        }
#ifdef VK_ENABLE_BETA_EXTENSIONS
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR && p.Type() != OutputType::json) {
            const VkPhysicalDevicePortabilitySubsetFeaturesKHR* props = (const VkPhysicalDevicePortabilitySubsetFeaturesKHR*)structure;
            DumpVkPhysicalDevicePortabilitySubsetFeaturesKHR(p, "VkPhysicalDevicePortabilitySubsetFeaturesKHR", *props);
            p.AddNewline();
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR) {
            const VkPhysicalDevicePresentIdFeaturesKHR* props = (const VkPhysicalDevicePresentIdFeaturesKHR*)structure;
            DumpVkPhysicalDevicePresentIdFeaturesKHR(p, "VkPhysicalDevicePresentIdFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT) {
            const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT* props = (const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT*)structure;
            DumpVkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT(p, "VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR) {
            const VkPhysicalDevicePresentWaitFeaturesKHR* props = (const VkPhysicalDevicePresentWaitFeaturesKHR*)structure;
            DumpVkPhysicalDevicePresentWaitFeaturesKHR(p, "VkPhysicalDevicePresentWaitFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT) {
            const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT* props = (const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT*)structure;
            DumpVkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(p, "VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT) {
            const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT* props = (const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT*)structure;
            DumpVkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(p, "VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES) {
            const VkPhysicalDevicePrivateDataFeatures* props = (const VkPhysicalDevicePrivateDataFeatures*)structure;
            DumpVkPhysicalDevicePrivateDataFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDevicePrivateDataFeatures":"VkPhysicalDevicePrivateDataFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES) {
            const VkPhysicalDeviceProtectedMemoryFeatures* props = (const VkPhysicalDeviceProtectedMemoryFeatures*)structure;
            DumpVkPhysicalDeviceProtectedMemoryFeatures(p, "VkPhysicalDeviceProtectedMemoryFeatures", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT) {
            const VkPhysicalDeviceProvokingVertexFeaturesEXT* props = (const VkPhysicalDeviceProvokingVertexFeaturesEXT*)structure;
            DumpVkPhysicalDeviceProvokingVertexFeaturesEXT(p, "VkPhysicalDeviceProvokingVertexFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT) {
            const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT* props = (const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT*)structure;
            DumpVkPhysicalDeviceRGBA10X6FormatsFeaturesEXT(p, "VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT) {
            const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT* props = (const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT*)structure;
            DumpVkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(p, "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR) {
            const VkPhysicalDeviceRayQueryFeaturesKHR* props = (const VkPhysicalDeviceRayQueryFeaturesKHR*)structure;
            DumpVkPhysicalDeviceRayQueryFeaturesKHR(p, "VkPhysicalDeviceRayQueryFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR) {
            const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* props = (const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR*)structure;
            DumpVkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(p, "VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR) {
            const VkPhysicalDeviceRayTracingPipelineFeaturesKHR* props = (const VkPhysicalDeviceRayTracingPipelineFeaturesKHR*)structure;
            DumpVkPhysicalDeviceRayTracingPipelineFeaturesKHR(p, "VkPhysicalDeviceRayTracingPipelineFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR) {
            const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* props = (const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR*)structure;
            DumpVkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(p, "VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT) {
            const VkPhysicalDeviceRobustness2FeaturesEXT* props = (const VkPhysicalDeviceRobustness2FeaturesEXT*)structure;
            DumpVkPhysicalDeviceRobustness2FeaturesEXT(p, "VkPhysicalDeviceRobustness2FeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES) {
            const VkPhysicalDeviceSamplerYcbcrConversionFeatures* props = (const VkPhysicalDeviceSamplerYcbcrConversionFeatures*)structure;
            DumpVkPhysicalDeviceSamplerYcbcrConversionFeatures(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDeviceSamplerYcbcrConversionFeatures":"VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES) {
            const VkPhysicalDeviceScalarBlockLayoutFeatures* props = (const VkPhysicalDeviceScalarBlockLayoutFeatures*)structure;
            DumpVkPhysicalDeviceScalarBlockLayoutFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceScalarBlockLayoutFeatures":"VkPhysicalDeviceScalarBlockLayoutFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES) {
            const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures* props = (const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures*)structure;
            DumpVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures":"VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT) {
            const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT* props = (const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT*)structure;
            DumpVkPhysicalDeviceShaderAtomicFloat2FeaturesEXT(p, "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT) {
            const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* props = (const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT*)structure;
            DumpVkPhysicalDeviceShaderAtomicFloatFeaturesEXT(p, "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES) {
            const VkPhysicalDeviceShaderAtomicInt64Features* props = (const VkPhysicalDeviceShaderAtomicInt64Features*)structure;
            DumpVkPhysicalDeviceShaderAtomicInt64Features(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceShaderAtomicInt64Features":"VkPhysicalDeviceShaderAtomicInt64FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR) {
            const VkPhysicalDeviceShaderClockFeaturesKHR* props = (const VkPhysicalDeviceShaderClockFeaturesKHR*)structure;
            DumpVkPhysicalDeviceShaderClockFeaturesKHR(p, "VkPhysicalDeviceShaderClockFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES) {
            const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures* props = (const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures*)structure;
            DumpVkPhysicalDeviceShaderDemoteToHelperInvocationFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures":"VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES) {
            const VkPhysicalDeviceShaderDrawParametersFeatures* props = (const VkPhysicalDeviceShaderDrawParametersFeatures*)structure;
            DumpVkPhysicalDeviceShaderDrawParametersFeatures(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDeviceShaderDrawParametersFeatures":"VkPhysicalDeviceShaderDrawParameterFeatures", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES) {
            const VkPhysicalDeviceShaderExpectAssumeFeatures* props = (const VkPhysicalDeviceShaderExpectAssumeFeatures*)structure;
            DumpVkPhysicalDeviceShaderExpectAssumeFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceShaderExpectAssumeFeatures":"VkPhysicalDeviceShaderExpectAssumeFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES) {
            const VkPhysicalDeviceShaderFloat16Int8Features* props = (const VkPhysicalDeviceShaderFloat16Int8Features*)structure;
            DumpVkPhysicalDeviceShaderFloat16Int8Features(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceShaderFloat16Int8Features":"VkPhysicalDeviceFloat16Int8FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES) {
            const VkPhysicalDeviceShaderFloatControls2Features* props = (const VkPhysicalDeviceShaderFloatControls2Features*)structure;
            DumpVkPhysicalDeviceShaderFloatControls2Features(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceShaderFloatControls2Features":"VkPhysicalDeviceShaderFloatControls2FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT) {
            const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* props = (const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT*)structure;
            DumpVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(p, "VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES) {
            const VkPhysicalDeviceShaderIntegerDotProductFeatures* props = (const VkPhysicalDeviceShaderIntegerDotProductFeatures*)structure;
            DumpVkPhysicalDeviceShaderIntegerDotProductFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceShaderIntegerDotProductFeatures":"VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR) {
            const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR* props = (const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR*)structure;
            DumpVkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR(p, "VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT) {
            const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT* props = (const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT*)structure;
            DumpVkPhysicalDeviceShaderModuleIdentifierFeaturesEXT(p, "VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT) {
            const VkPhysicalDeviceShaderObjectFeaturesEXT* props = (const VkPhysicalDeviceShaderObjectFeaturesEXT*)structure;
            DumpVkPhysicalDeviceShaderObjectFeaturesEXT(p, "VkPhysicalDeviceShaderObjectFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR) {
            const VkPhysicalDeviceShaderQuadControlFeaturesKHR* props = (const VkPhysicalDeviceShaderQuadControlFeaturesKHR*)structure;
            DumpVkPhysicalDeviceShaderQuadControlFeaturesKHR(p, "VkPhysicalDeviceShaderQuadControlFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR) {
            const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR* props = (const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR*)structure;
            DumpVkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR(p, "VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT) {
            const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT* props = (const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT*)structure;
            DumpVkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT(p, "VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES) {
            const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures* props = (const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures*)structure;
            DumpVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures":"VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES) {
            const VkPhysicalDeviceShaderSubgroupRotateFeatures* props = (const VkPhysicalDeviceShaderSubgroupRotateFeatures*)structure;
            DumpVkPhysicalDeviceShaderSubgroupRotateFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceShaderSubgroupRotateFeatures":"VkPhysicalDeviceShaderSubgroupRotateFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR) {
            const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* props = (const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR*)structure;
            DumpVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(p, "VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES) {
            const VkPhysicalDeviceShaderTerminateInvocationFeatures* props = (const VkPhysicalDeviceShaderTerminateInvocationFeatures*)structure;
            DumpVkPhysicalDeviceShaderTerminateInvocationFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceShaderTerminateInvocationFeatures":"VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT) {
            const VkPhysicalDeviceShaderTileImageFeaturesEXT* props = (const VkPhysicalDeviceShaderTileImageFeaturesEXT*)structure;
            DumpVkPhysicalDeviceShaderTileImageFeaturesEXT(p, "VkPhysicalDeviceShaderTileImageFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES) {
            const VkPhysicalDeviceSubgroupSizeControlFeatures* props = (const VkPhysicalDeviceSubgroupSizeControlFeatures*)structure;
            DumpVkPhysicalDeviceSubgroupSizeControlFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceSubgroupSizeControlFeatures":"VkPhysicalDeviceSubgroupSizeControlFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT) {
            const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT* props = (const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT*)structure;
            DumpVkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT(p, "VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT) {
            const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT* props = (const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT*)structure;
            DumpVkPhysicalDeviceSwapchainMaintenance1FeaturesEXT(p, "VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES) {
            const VkPhysicalDeviceSynchronization2Features* props = (const VkPhysicalDeviceSynchronization2Features*)structure;
            DumpVkPhysicalDeviceSynchronization2Features(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceSynchronization2Features":"VkPhysicalDeviceSynchronization2FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT) {
            const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* props = (const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT*)structure;
            DumpVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(p, "VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES) {
            const VkPhysicalDeviceTextureCompressionASTCHDRFeatures* props = (const VkPhysicalDeviceTextureCompressionASTCHDRFeatures*)structure;
            DumpVkPhysicalDeviceTextureCompressionASTCHDRFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceTextureCompressionASTCHDRFeatures":"VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES) {
            const VkPhysicalDeviceTimelineSemaphoreFeatures* props = (const VkPhysicalDeviceTimelineSemaphoreFeatures*)structure;
            DumpVkPhysicalDeviceTimelineSemaphoreFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceTimelineSemaphoreFeatures":"VkPhysicalDeviceTimelineSemaphoreFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT) {
            const VkPhysicalDeviceTransformFeedbackFeaturesEXT* props = (const VkPhysicalDeviceTransformFeedbackFeaturesEXT*)structure;
            DumpVkPhysicalDeviceTransformFeedbackFeaturesEXT(p, "VkPhysicalDeviceTransformFeedbackFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES) {
            const VkPhysicalDeviceUniformBufferStandardLayoutFeatures* props = (const VkPhysicalDeviceUniformBufferStandardLayoutFeatures*)structure;
            DumpVkPhysicalDeviceUniformBufferStandardLayoutFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceUniformBufferStandardLayoutFeatures":"VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES) {
            const VkPhysicalDeviceVariablePointersFeatures* props = (const VkPhysicalDeviceVariablePointersFeatures*)structure;
            DumpVkPhysicalDeviceVariablePointersFeatures(p, gpu.api_version >= VK_API_VERSION_1_1 ?"VkPhysicalDeviceVariablePointersFeatures":"VkPhysicalDeviceVariablePointersFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES) {
            const VkPhysicalDeviceVertexAttributeDivisorFeatures* props = (const VkPhysicalDeviceVertexAttributeDivisorFeatures*)structure;
            DumpVkPhysicalDeviceVertexAttributeDivisorFeatures(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkPhysicalDeviceVertexAttributeDivisorFeatures":"VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT) {
            const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT* props = (const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT*)structure;
            DumpVkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT(p, "VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT) {
            const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* props = (const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT*)structure;
            DumpVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(p, "VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR) {
            const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR* props = (const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR*)structure;
            DumpVkPhysicalDeviceVideoEncodeAV1FeaturesKHR(p, "VkPhysicalDeviceVideoEncodeAV1FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR) {
            const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR* props = (const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR*)structure;
            DumpVkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR(p, "VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR) {
            const VkPhysicalDeviceVideoMaintenance1FeaturesKHR* props = (const VkPhysicalDeviceVideoMaintenance1FeaturesKHR*)structure;
            DumpVkPhysicalDeviceVideoMaintenance1FeaturesKHR(p, "VkPhysicalDeviceVideoMaintenance1FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR) {
            const VkPhysicalDeviceVideoMaintenance2FeaturesKHR* props = (const VkPhysicalDeviceVideoMaintenance2FeaturesKHR*)structure;
            DumpVkPhysicalDeviceVideoMaintenance2FeaturesKHR(p, "VkPhysicalDeviceVideoMaintenance2FeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES) {
            const VkPhysicalDeviceVulkan11Features* props = (const VkPhysicalDeviceVulkan11Features*)structure;
            DumpVkPhysicalDeviceVulkan11Features(p, "VkPhysicalDeviceVulkan11Features", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES) {
            const VkPhysicalDeviceVulkan12Features* props = (const VkPhysicalDeviceVulkan12Features*)structure;
            DumpVkPhysicalDeviceVulkan12Features(p, "VkPhysicalDeviceVulkan12Features", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES) {
            const VkPhysicalDeviceVulkan13Features* props = (const VkPhysicalDeviceVulkan13Features*)structure;
            DumpVkPhysicalDeviceVulkan13Features(p, "VkPhysicalDeviceVulkan13Features", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES) {
            const VkPhysicalDeviceVulkan14Features* props = (const VkPhysicalDeviceVulkan14Features*)structure;
            DumpVkPhysicalDeviceVulkan14Features(p, "VkPhysicalDeviceVulkan14Features", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES) {
            const VkPhysicalDeviceVulkanMemoryModelFeatures* props = (const VkPhysicalDeviceVulkanMemoryModelFeatures*)structure;
            DumpVkPhysicalDeviceVulkanMemoryModelFeatures(p, gpu.api_version >= VK_API_VERSION_1_2 ?"VkPhysicalDeviceVulkanMemoryModelFeatures":"VkPhysicalDeviceVulkanMemoryModelFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR) {
            const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* props = (const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR*)structure;
            DumpVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(p, "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT) {
            const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* props = (const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT*)structure;
            DumpVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(p, "VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT) {
            const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* props = (const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT*)structure;
            DumpVkPhysicalDeviceYcbcrImageArraysFeaturesEXT(p, "VkPhysicalDeviceYcbcrImageArraysFeaturesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES) {
            const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures* props = (const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures*)structure;
            DumpVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures":"VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR", *props);
            p.AddNewline();
        }
        place = structure->pNext;
    }
}

bool prepare_phys_device_features2_twocall_chain_vectors(std::unique_ptr<phys_device_features2_chain>& chain) {
    (void)chain;
    return false;
}
struct surface_capabilities2_chain {
    surface_capabilities2_chain() = default;
    surface_capabilities2_chain(const surface_capabilities2_chain &) = delete;
    surface_capabilities2_chain& operator=(const surface_capabilities2_chain &) = delete;
    surface_capabilities2_chain(surface_capabilities2_chain &&) = delete;
    surface_capabilities2_chain& operator=(surface_capabilities2_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkSharedPresentSurfaceCapabilitiesKHR SharedPresentSurfaceCapabilitiesKHR{};
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkSurfaceCapabilitiesFullScreenExclusiveEXT SurfaceCapabilitiesFullScreenExclusiveEXT{};
#endif  // VK_USE_PLATFORM_WIN32_KHR
    VkSurfaceProtectedCapabilitiesKHR SurfaceProtectedCapabilitiesKHR{};
    void initialize_chain(AppInstance &inst, AppGpu &gpu ) noexcept {
        SharedPresentSurfaceCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        SurfaceCapabilitiesFullScreenExclusiveEXT.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        SurfaceProtectedCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR;
        std::vector<VkBaseOutStructure*> chain_members{};
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&SharedPresentSurfaceCapabilitiesKHR));
#ifdef VK_USE_PLATFORM_WIN32_KHR
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&SurfaceCapabilitiesFullScreenExclusiveEXT));
#endif  // VK_USE_PLATFORM_WIN32_KHR
        if (inst.CheckExtensionEnabled(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&SurfaceProtectedCapabilitiesKHR));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_surface_capabilities2_chain(VkSurfaceCapabilities2KHR& start, std::unique_ptr<surface_capabilities2_chain>& chain, AppInstance &inst,AppGpu &gpu){
    chain = std::unique_ptr<surface_capabilities2_chain>(new surface_capabilities2_chain());
    chain->initialize_chain(inst,gpu);
    start.pNext = chain->start_of_chain;
};

void chain_iterator_surface_capabilities2(Printer &p, AppInstance &inst, AppGpu &gpu, const void * place) {
    while (place) {
        const VkBaseOutStructure *structure = (const VkBaseOutStructure *)place;
        p.SetSubHeader();
        if (structure->sType == VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR) {
            const VkSharedPresentSurfaceCapabilitiesKHR* props = (const VkSharedPresentSurfaceCapabilitiesKHR*)structure;
            DumpVkSharedPresentSurfaceCapabilitiesKHR(p, "VkSharedPresentSurfaceCapabilitiesKHR", *props);
            p.AddNewline();
        }
#ifdef VK_USE_PLATFORM_WIN32_KHR
        if (structure->sType == VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT) {
            const VkSurfaceCapabilitiesFullScreenExclusiveEXT* props = (const VkSurfaceCapabilitiesFullScreenExclusiveEXT*)structure;
            DumpVkSurfaceCapabilitiesFullScreenExclusiveEXT(p, "VkSurfaceCapabilitiesFullScreenExclusiveEXT", *props);
            p.AddNewline();
        }
#endif  // VK_USE_PLATFORM_WIN32_KHR
        if (structure->sType == VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR) {
            const VkSurfaceProtectedCapabilitiesKHR* props = (const VkSurfaceProtectedCapabilitiesKHR*)structure;
            DumpVkSurfaceProtectedCapabilitiesKHR(p, "VkSurfaceProtectedCapabilitiesKHR", *props);
            p.AddNewline();
        }
        place = structure->pNext;
    }
}

bool prepare_surface_capabilities2_twocall_chain_vectors(std::unique_ptr<surface_capabilities2_chain>& chain) {
    (void)chain;
    return false;
}
struct format_properties2_chain {
    format_properties2_chain() = default;
    format_properties2_chain(const format_properties2_chain &) = delete;
    format_properties2_chain& operator=(const format_properties2_chain &) = delete;
    format_properties2_chain(format_properties2_chain &&) = delete;
    format_properties2_chain& operator=(format_properties2_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkFormatProperties3 FormatProperties3{};
    VkSubpassResolvePerformanceQueryEXT SubpassResolvePerformanceQueryEXT{};
    void initialize_chain(AppGpu &gpu ) noexcept {
        FormatProperties3.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3;
        SubpassResolvePerformanceQueryEXT.sType = VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT;
        std::vector<VkBaseOutStructure*> chain_members{};
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_3 ))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&FormatProperties3));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&SubpassResolvePerformanceQueryEXT));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_format_properties2_chain(VkFormatProperties2& start, std::unique_ptr<format_properties2_chain>& chain, AppGpu &gpu){
    chain = std::unique_ptr<format_properties2_chain>(new format_properties2_chain());
    chain->initialize_chain(gpu);
    start.pNext = chain->start_of_chain;
};

void chain_iterator_format_properties2(Printer &p, AppGpu &gpu, const void * place) {
    while (place) {
        const VkBaseOutStructure *structure = (const VkBaseOutStructure *)place;
        p.SetSubHeader();
        if (structure->sType == VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3) {
            const VkFormatProperties3* props = (const VkFormatProperties3*)structure;
            DumpVkFormatProperties3(p, gpu.api_version >= VK_API_VERSION_1_3 ?"VkFormatProperties3":"VkFormatProperties3KHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT) {
            const VkSubpassResolvePerformanceQueryEXT* props = (const VkSubpassResolvePerformanceQueryEXT*)structure;
            DumpVkSubpassResolvePerformanceQueryEXT(p, "VkSubpassResolvePerformanceQueryEXT", *props);
            p.AddNewline();
        }
        place = structure->pNext;
    }
}

bool prepare_format_properties2_twocall_chain_vectors(std::unique_ptr<format_properties2_chain>& chain) {
    (void)chain;
    return false;
}
struct queue_properties2_chain {
    queue_properties2_chain() = default;
    queue_properties2_chain(const queue_properties2_chain &) = delete;
    queue_properties2_chain& operator=(const queue_properties2_chain &) = delete;
    queue_properties2_chain(queue_properties2_chain &&) = delete;
    queue_properties2_chain& operator=(queue_properties2_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkQueueFamilyGlobalPriorityProperties QueueFamilyGlobalPriorityProperties{};
    VkQueueFamilyQueryResultStatusPropertiesKHR QueueFamilyQueryResultStatusPropertiesKHR{};
    VkQueueFamilyVideoPropertiesKHR QueueFamilyVideoPropertiesKHR{};
    void initialize_chain(AppGpu &gpu ) noexcept {
        QueueFamilyGlobalPriorityProperties.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES;
        QueueFamilyQueryResultStatusPropertiesKHR.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR;
        QueueFamilyVideoPropertiesKHR.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR;
        std::vector<VkBaseOutStructure*> chain_members{};
        if ((gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME)
         || gpu.CheckPhysicalDeviceExtensionIncluded(VK_EXT_GLOBAL_PRIORITY_QUERY_EXTENSION_NAME))
            && (gpu.api_version < VK_API_VERSION_1_4 ))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&QueueFamilyGlobalPriorityProperties));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&QueueFamilyQueryResultStatusPropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&QueueFamilyVideoPropertiesKHR));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_queue_properties2_chain(VkQueueFamilyProperties2& start, std::unique_ptr<queue_properties2_chain>& chain, AppGpu &gpu){
    chain = std::unique_ptr<queue_properties2_chain>(new queue_properties2_chain());
    chain->initialize_chain(gpu);
    start.pNext = chain->start_of_chain;
};

void chain_iterator_queue_properties2(Printer &p, AppGpu &gpu, const void * place) {
    while (place) {
        const VkBaseOutStructure *structure = (const VkBaseOutStructure *)place;
        p.SetSubHeader();
        if (structure->sType == VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES) {
            const VkQueueFamilyGlobalPriorityProperties* props = (const VkQueueFamilyGlobalPriorityProperties*)structure;
            DumpVkQueueFamilyGlobalPriorityProperties(p, gpu.api_version >= VK_API_VERSION_1_4 ?"VkQueueFamilyGlobalPriorityProperties":"VkQueueFamilyGlobalPriorityPropertiesEXT", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR) {
            const VkQueueFamilyQueryResultStatusPropertiesKHR* props = (const VkQueueFamilyQueryResultStatusPropertiesKHR*)structure;
            DumpVkQueueFamilyQueryResultStatusPropertiesKHR(p, "VkQueueFamilyQueryResultStatusPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR) {
            const VkQueueFamilyVideoPropertiesKHR* props = (const VkQueueFamilyVideoPropertiesKHR*)structure;
            DumpVkQueueFamilyVideoPropertiesKHR(p, "VkQueueFamilyVideoPropertiesKHR", *props);
            p.AddNewline();
        }
        place = structure->pNext;
    }
}

bool prepare_queue_properties2_twocall_chain_vectors(std::unique_ptr<queue_properties2_chain>& chain) {
    (void)chain;
    return false;
}
struct video_profile_info_chain {
    video_profile_info_chain() = default;
    video_profile_info_chain(const video_profile_info_chain &) = delete;
    video_profile_info_chain& operator=(const video_profile_info_chain &) = delete;
    video_profile_info_chain(video_profile_info_chain &&) = delete;
    video_profile_info_chain& operator=(video_profile_info_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkVideoDecodeAV1ProfileInfoKHR VideoDecodeAV1ProfileInfoKHR{};
    VkVideoDecodeH264ProfileInfoKHR VideoDecodeH264ProfileInfoKHR{};
    VkVideoDecodeH265ProfileInfoKHR VideoDecodeH265ProfileInfoKHR{};
    VkVideoDecodeUsageInfoKHR VideoDecodeUsageInfoKHR{};
    VkVideoEncodeAV1ProfileInfoKHR VideoEncodeAV1ProfileInfoKHR{};
    VkVideoEncodeH264ProfileInfoKHR VideoEncodeH264ProfileInfoKHR{};
    VkVideoEncodeH265ProfileInfoKHR VideoEncodeH265ProfileInfoKHR{};
    VkVideoEncodeUsageInfoKHR VideoEncodeUsageInfoKHR{};
    void initialize_chain(AppGpu &gpu ) noexcept {
        VideoDecodeAV1ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR;
        VideoDecodeH264ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR;
        VideoDecodeH265ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR;
        VideoDecodeUsageInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR;
        VideoEncodeAV1ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR;
        VideoEncodeH264ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR;
        VideoEncodeH265ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR;
        VideoEncodeUsageInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR;
        std::vector<VkBaseOutStructure*> chain_members{};
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_AV1_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoDecodeAV1ProfileInfoKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoDecodeH264ProfileInfoKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoDecodeH265ProfileInfoKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoDecodeUsageInfoKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_AV1_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeAV1ProfileInfoKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeH264ProfileInfoKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeH265ProfileInfoKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeUsageInfoKHR));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_video_profile_info_chain(VkVideoProfileInfoKHR& start, std::unique_ptr<video_profile_info_chain>& chain, AppGpu &gpu){
    chain = std::unique_ptr<video_profile_info_chain>(new video_profile_info_chain());
    chain->initialize_chain(gpu);
    start.pNext = chain->start_of_chain;
};

void chain_iterator_video_profile_info(Printer &p, AppGpu &gpu, const void * place) {
    while (place) {
        const VkBaseOutStructure *structure = (const VkBaseOutStructure *)place;
        p.SetSubHeader();
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR) {
            const VkVideoDecodeAV1ProfileInfoKHR* props = (const VkVideoDecodeAV1ProfileInfoKHR*)structure;
            DumpVkVideoDecodeAV1ProfileInfoKHR(p, "VkVideoDecodeAV1ProfileInfoKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR) {
            const VkVideoDecodeH264ProfileInfoKHR* props = (const VkVideoDecodeH264ProfileInfoKHR*)structure;
            DumpVkVideoDecodeH264ProfileInfoKHR(p, "VkVideoDecodeH264ProfileInfoKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR) {
            const VkVideoDecodeH265ProfileInfoKHR* props = (const VkVideoDecodeH265ProfileInfoKHR*)structure;
            DumpVkVideoDecodeH265ProfileInfoKHR(p, "VkVideoDecodeH265ProfileInfoKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR) {
            const VkVideoDecodeUsageInfoKHR* props = (const VkVideoDecodeUsageInfoKHR*)structure;
            DumpVkVideoDecodeUsageInfoKHR(p, "VkVideoDecodeUsageInfoKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR) {
            const VkVideoEncodeAV1ProfileInfoKHR* props = (const VkVideoEncodeAV1ProfileInfoKHR*)structure;
            DumpVkVideoEncodeAV1ProfileInfoKHR(p, "VkVideoEncodeAV1ProfileInfoKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR) {
            const VkVideoEncodeH264ProfileInfoKHR* props = (const VkVideoEncodeH264ProfileInfoKHR*)structure;
            DumpVkVideoEncodeH264ProfileInfoKHR(p, "VkVideoEncodeH264ProfileInfoKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR) {
            const VkVideoEncodeH265ProfileInfoKHR* props = (const VkVideoEncodeH265ProfileInfoKHR*)structure;
            DumpVkVideoEncodeH265ProfileInfoKHR(p, "VkVideoEncodeH265ProfileInfoKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR) {
            const VkVideoEncodeUsageInfoKHR* props = (const VkVideoEncodeUsageInfoKHR*)structure;
            DumpVkVideoEncodeUsageInfoKHR(p, "VkVideoEncodeUsageInfoKHR", *props);
            p.AddNewline();
        }
        place = structure->pNext;
    }
}

bool prepare_video_profile_info_twocall_chain_vectors(std::unique_ptr<video_profile_info_chain>& chain) {
    (void)chain;
    return false;
}
struct video_capabilities_chain {
    video_capabilities_chain() = default;
    video_capabilities_chain(const video_capabilities_chain &) = delete;
    video_capabilities_chain& operator=(const video_capabilities_chain &) = delete;
    video_capabilities_chain(video_capabilities_chain &&) = delete;
    video_capabilities_chain& operator=(video_capabilities_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkVideoDecodeAV1CapabilitiesKHR VideoDecodeAV1CapabilitiesKHR{};
    VkVideoDecodeCapabilitiesKHR VideoDecodeCapabilitiesKHR{};
    VkVideoDecodeH264CapabilitiesKHR VideoDecodeH264CapabilitiesKHR{};
    VkVideoDecodeH265CapabilitiesKHR VideoDecodeH265CapabilitiesKHR{};
    VkVideoEncodeAV1CapabilitiesKHR VideoEncodeAV1CapabilitiesKHR{};
    VkVideoEncodeAV1QuantizationMapCapabilitiesKHR VideoEncodeAV1QuantizationMapCapabilitiesKHR{};
    VkVideoEncodeCapabilitiesKHR VideoEncodeCapabilitiesKHR{};
    VkVideoEncodeH264CapabilitiesKHR VideoEncodeH264CapabilitiesKHR{};
    VkVideoEncodeH264QuantizationMapCapabilitiesKHR VideoEncodeH264QuantizationMapCapabilitiesKHR{};
    VkVideoEncodeH265CapabilitiesKHR VideoEncodeH265CapabilitiesKHR{};
    VkVideoEncodeH265QuantizationMapCapabilitiesKHR VideoEncodeH265QuantizationMapCapabilitiesKHR{};
    VkVideoEncodeQuantizationMapCapabilitiesKHR VideoEncodeQuantizationMapCapabilitiesKHR{};
    void initialize_chain(AppGpu &gpu ) noexcept {
        VideoDecodeAV1CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR;
        VideoDecodeCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR;
        VideoDecodeH264CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR;
        VideoDecodeH265CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR;
        VideoEncodeAV1CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR;
        VideoEncodeAV1QuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR;
        VideoEncodeCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR;
        VideoEncodeH264CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR;
        VideoEncodeH264QuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR;
        VideoEncodeH265CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR;
        VideoEncodeH265QuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR;
        VideoEncodeQuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR;
        std::vector<VkBaseOutStructure*> chain_members{};
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_AV1_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoDecodeAV1CapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoDecodeCapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoDecodeH264CapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoDecodeH265CapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_AV1_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeAV1CapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeAV1QuantizationMapCapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeCapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeH264CapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeH264QuantizationMapCapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeH265CapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeH265QuantizationMapCapabilitiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoEncodeQuantizationMapCapabilitiesKHR));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_video_capabilities_chain(VkVideoCapabilitiesKHR& start, std::unique_ptr<video_capabilities_chain>& chain, AppGpu &gpu){
    chain = std::unique_ptr<video_capabilities_chain>(new video_capabilities_chain());
    chain->initialize_chain(gpu);
    start.pNext = chain->start_of_chain;
};

void chain_iterator_video_capabilities(Printer &p, AppGpu &gpu, const void * place) {
    while (place) {
        const VkBaseOutStructure *structure = (const VkBaseOutStructure *)place;
        p.SetSubHeader();
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR) {
            const VkVideoDecodeAV1CapabilitiesKHR* props = (const VkVideoDecodeAV1CapabilitiesKHR*)structure;
            DumpVkVideoDecodeAV1CapabilitiesKHR(p, "VkVideoDecodeAV1CapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR) {
            const VkVideoDecodeCapabilitiesKHR* props = (const VkVideoDecodeCapabilitiesKHR*)structure;
            DumpVkVideoDecodeCapabilitiesKHR(p, "VkVideoDecodeCapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR) {
            const VkVideoDecodeH264CapabilitiesKHR* props = (const VkVideoDecodeH264CapabilitiesKHR*)structure;
            DumpVkVideoDecodeH264CapabilitiesKHR(p, "VkVideoDecodeH264CapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR) {
            const VkVideoDecodeH265CapabilitiesKHR* props = (const VkVideoDecodeH265CapabilitiesKHR*)structure;
            DumpVkVideoDecodeH265CapabilitiesKHR(p, "VkVideoDecodeH265CapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR) {
            const VkVideoEncodeAV1CapabilitiesKHR* props = (const VkVideoEncodeAV1CapabilitiesKHR*)structure;
            DumpVkVideoEncodeAV1CapabilitiesKHR(p, "VkVideoEncodeAV1CapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR) {
            const VkVideoEncodeAV1QuantizationMapCapabilitiesKHR* props = (const VkVideoEncodeAV1QuantizationMapCapabilitiesKHR*)structure;
            DumpVkVideoEncodeAV1QuantizationMapCapabilitiesKHR(p, "VkVideoEncodeAV1QuantizationMapCapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR) {
            const VkVideoEncodeCapabilitiesKHR* props = (const VkVideoEncodeCapabilitiesKHR*)structure;
            DumpVkVideoEncodeCapabilitiesKHR(p, "VkVideoEncodeCapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR) {
            const VkVideoEncodeH264CapabilitiesKHR* props = (const VkVideoEncodeH264CapabilitiesKHR*)structure;
            DumpVkVideoEncodeH264CapabilitiesKHR(p, "VkVideoEncodeH264CapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR) {
            const VkVideoEncodeH264QuantizationMapCapabilitiesKHR* props = (const VkVideoEncodeH264QuantizationMapCapabilitiesKHR*)structure;
            DumpVkVideoEncodeH264QuantizationMapCapabilitiesKHR(p, "VkVideoEncodeH264QuantizationMapCapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR) {
            const VkVideoEncodeH265CapabilitiesKHR* props = (const VkVideoEncodeH265CapabilitiesKHR*)structure;
            DumpVkVideoEncodeH265CapabilitiesKHR(p, "VkVideoEncodeH265CapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR) {
            const VkVideoEncodeH265QuantizationMapCapabilitiesKHR* props = (const VkVideoEncodeH265QuantizationMapCapabilitiesKHR*)structure;
            DumpVkVideoEncodeH265QuantizationMapCapabilitiesKHR(p, "VkVideoEncodeH265QuantizationMapCapabilitiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR) {
            const VkVideoEncodeQuantizationMapCapabilitiesKHR* props = (const VkVideoEncodeQuantizationMapCapabilitiesKHR*)structure;
            DumpVkVideoEncodeQuantizationMapCapabilitiesKHR(p, "VkVideoEncodeQuantizationMapCapabilitiesKHR", *props);
            p.AddNewline();
        }
        place = structure->pNext;
    }
}

bool prepare_video_capabilities_twocall_chain_vectors(std::unique_ptr<video_capabilities_chain>& chain) {
    (void)chain;
    return false;
}
struct video_format_properties_chain {
    video_format_properties_chain() = default;
    video_format_properties_chain(const video_format_properties_chain &) = delete;
    video_format_properties_chain& operator=(const video_format_properties_chain &) = delete;
    video_format_properties_chain(video_format_properties_chain &&) = delete;
    video_format_properties_chain& operator=(video_format_properties_chain &&) = delete;
    void* start_of_chain = nullptr;
    VkVideoFormatAV1QuantizationMapPropertiesKHR VideoFormatAV1QuantizationMapPropertiesKHR{};
    VkVideoFormatH265QuantizationMapPropertiesKHR VideoFormatH265QuantizationMapPropertiesKHR{};
    VkVideoFormatQuantizationMapPropertiesKHR VideoFormatQuantizationMapPropertiesKHR{};
    void initialize_chain(AppGpu &gpu ) noexcept {
        VideoFormatAV1QuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR;
        VideoFormatH265QuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR;
        VideoFormatQuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
        std::vector<VkBaseOutStructure*> chain_members{};
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoFormatAV1QuantizationMapPropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoFormatH265QuantizationMapPropertiesKHR));
        if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME))
            chain_members.push_back(reinterpret_cast<VkBaseOutStructure*>(&VideoFormatQuantizationMapPropertiesKHR));

        if (!chain_members.empty()) {
            for(size_t i = 0; i < chain_members.size() - 1; i++){
                chain_members[i]->pNext = chain_members[i + 1];
            }
            start_of_chain = chain_members[0];
        }
    }
};
void setup_video_format_properties_chain(VkVideoFormatPropertiesKHR& start, std::unique_ptr<video_format_properties_chain>& chain, AppGpu &gpu){
    chain = std::unique_ptr<video_format_properties_chain>(new video_format_properties_chain());
    chain->initialize_chain(gpu);
    start.pNext = chain->start_of_chain;
};

void chain_iterator_video_format_properties(Printer &p, AppGpu &gpu, const void * place) {
    while (place) {
        const VkBaseOutStructure *structure = (const VkBaseOutStructure *)place;
        p.SetSubHeader();
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR) {
            const VkVideoFormatAV1QuantizationMapPropertiesKHR* props = (const VkVideoFormatAV1QuantizationMapPropertiesKHR*)structure;
            DumpVkVideoFormatAV1QuantizationMapPropertiesKHR(p, "VkVideoFormatAV1QuantizationMapPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR) {
            const VkVideoFormatH265QuantizationMapPropertiesKHR* props = (const VkVideoFormatH265QuantizationMapPropertiesKHR*)structure;
            DumpVkVideoFormatH265QuantizationMapPropertiesKHR(p, "VkVideoFormatH265QuantizationMapPropertiesKHR", *props);
            p.AddNewline();
        }
        if (structure->sType == VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR) {
            const VkVideoFormatQuantizationMapPropertiesKHR* props = (const VkVideoFormatQuantizationMapPropertiesKHR*)structure;
            DumpVkVideoFormatQuantizationMapPropertiesKHR(p, "VkVideoFormatQuantizationMapPropertiesKHR", *props);
            p.AddNewline();
        }
        place = structure->pNext;
    }
}

bool prepare_video_format_properties_twocall_chain_vectors(std::unique_ptr<video_format_properties_chain>& chain) {
    (void)chain;
    return false;
}
bool operator==(const VkExtent2D & a, const VkExtent2D b);
bool operator==(const VkSurfaceCapabilities2EXT & a, const VkSurfaceCapabilities2EXT b);
bool operator==(const VkSurfaceCapabilities2KHR & a, const VkSurfaceCapabilities2KHR b);
bool operator==(const VkSurfaceCapabilitiesKHR & a, const VkSurfaceCapabilitiesKHR b);
bool operator==(const VkSurfaceFormat2KHR & a, const VkSurfaceFormat2KHR b);
bool operator==(const VkSurfaceFormatKHR & a, const VkSurfaceFormatKHR b);
bool operator==(const VkExtent2D & a, const VkExtent2D b) {
    return a.width == b.width
        && a.height == b.height;
}
bool operator==(const VkSurfaceCapabilities2EXT & a, const VkSurfaceCapabilities2EXT b) {
    return a.minImageCount == b.minImageCount
        && a.maxImageCount == b.maxImageCount
        && a.currentExtent == b.currentExtent
        && a.minImageExtent == b.minImageExtent
        && a.maxImageExtent == b.maxImageExtent
        && a.maxImageArrayLayers == b.maxImageArrayLayers
        && a.supportedTransforms == b.supportedTransforms
        && a.currentTransform == b.currentTransform
        && a.supportedCompositeAlpha == b.supportedCompositeAlpha
        && a.supportedUsageFlags == b.supportedUsageFlags
        && a.supportedSurfaceCounters == b.supportedSurfaceCounters;
}
bool operator==(const VkSurfaceCapabilities2KHR & a, const VkSurfaceCapabilities2KHR b) {
    return a.surfaceCapabilities == b.surfaceCapabilities;
}
bool operator==(const VkSurfaceCapabilitiesKHR & a, const VkSurfaceCapabilitiesKHR b) {
    return a.minImageCount == b.minImageCount
        && a.maxImageCount == b.maxImageCount
        && a.currentExtent == b.currentExtent
        && a.minImageExtent == b.minImageExtent
        && a.maxImageExtent == b.maxImageExtent
        && a.maxImageArrayLayers == b.maxImageArrayLayers
        && a.supportedTransforms == b.supportedTransforms
        && a.currentTransform == b.currentTransform
        && a.supportedCompositeAlpha == b.supportedCompositeAlpha
        && a.supportedUsageFlags == b.supportedUsageFlags;
}
bool operator==(const VkSurfaceFormat2KHR & a, const VkSurfaceFormat2KHR b) {
    return a.surfaceFormat == b.surfaceFormat;
}
bool operator==(const VkSurfaceFormatKHR & a, const VkSurfaceFormatKHR b) {
    return a.format == b.format
        && a.colorSpace == b.colorSpace;
}
std::ostream &operator<<(std::ostream &o, VkExtent3D &obj) {
    return o << "(" << obj.width << ',' << obj.height << ',' << obj.depth << ")";
}
auto format_ranges = std::array{
    FormatRange{0, nullptr, static_cast<VkFormat>(0), static_cast<VkFormat>(184)},
    FormatRange{VK_API_VERSION_1_1, nullptr, static_cast<VkFormat>(1000156000), static_cast<VkFormat>(1000156033)},
    FormatRange{0, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, static_cast<VkFormat>(1000156000), static_cast<VkFormat>(1000156033)},
    FormatRange{VK_API_VERSION_1_3, nullptr, static_cast<VkFormat>(1000330000), static_cast<VkFormat>(1000330003)},
    FormatRange{0, VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME, static_cast<VkFormat>(1000330000), static_cast<VkFormat>(1000330003)},
    FormatRange{VK_API_VERSION_1_3, nullptr, static_cast<VkFormat>(1000340000), static_cast<VkFormat>(1000340001)},
    FormatRange{0, VK_EXT_4444_FORMATS_EXTENSION_NAME, static_cast<VkFormat>(1000340000), static_cast<VkFormat>(1000340001)},
    FormatRange{VK_API_VERSION_1_3, nullptr, static_cast<VkFormat>(1000066000), static_cast<VkFormat>(1000066013)},
    FormatRange{0, VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME, static_cast<VkFormat>(1000066000), static_cast<VkFormat>(1000066013)},
    FormatRange{VK_API_VERSION_1_4, nullptr, static_cast<VkFormat>(1000470000), static_cast<VkFormat>(1000470001)},
    FormatRange{0, VK_KHR_MAINTENANCE_5_EXTENSION_NAME, static_cast<VkFormat>(1000470000), static_cast<VkFormat>(1000470001)},
    FormatRange{0, VK_IMG_FORMAT_PVRTC_EXTENSION_NAME, static_cast<VkFormat>(1000054000), static_cast<VkFormat>(1000054007)},
    FormatRange{0, VK_NV_OPTICAL_FLOW_EXTENSION_NAME, static_cast<VkFormat>(1000464000), static_cast<VkFormat>(1000464000)},
};

bool is_video_format_same(const VkVideoFormatPropertiesKHR &format_a, const VkVideoFormatPropertiesKHR &format_b) {
    auto a = reinterpret_cast<const VkBaseInStructure*>(&format_a);
    auto b = reinterpret_cast<const VkBaseInStructure*>(&format_b);
    bool same = true;
    while (same && a != nullptr && b != nullptr) {
        if (a->sType != b->sType) {
            // Structure type mismatch (extension structures are expected to be chained in the same order)
            same = false;
        } else {
            switch (a->sType) {
                case VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR:
                    same = same && memcmp(reinterpret_cast<const char*>(a) + sizeof(VkBaseInStructure),
                                          reinterpret_cast<const char*>(b) + sizeof(VkBaseInStructure),
                                          sizeof(VkVideoFormatPropertiesKHR) - sizeof(VkBaseInStructure)) == 0;
                    break;
                case VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR:
                    same = same && memcmp(reinterpret_cast<const char*>(a) + sizeof(VkBaseInStructure),
                                          reinterpret_cast<const char*>(b) + sizeof(VkBaseInStructure),
                                          sizeof(VkVideoFormatAV1QuantizationMapPropertiesKHR) - sizeof(VkBaseInStructure)) == 0;
                    break;
                case VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR:
                    same = same && memcmp(reinterpret_cast<const char*>(a) + sizeof(VkBaseInStructure),
                                          reinterpret_cast<const char*>(b) + sizeof(VkBaseInStructure),
                                          sizeof(VkVideoFormatH265QuantizationMapPropertiesKHR) - sizeof(VkBaseInStructure)) == 0;
                    break;
                case VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR:
                    same = same && memcmp(reinterpret_cast<const char*>(a) + sizeof(VkBaseInStructure),
                                          reinterpret_cast<const char*>(b) + sizeof(VkBaseInStructure),
                                          sizeof(VkVideoFormatQuantizationMapPropertiesKHR) - sizeof(VkBaseInStructure)) == 0;
                    break;
                default:
                    // Unexpected structure type
                    same = false;
                    break;
            }
        }
        a = a->pNext;
        b = b->pNext;
    }
    return same;
}

std::vector<std::unique_ptr<AppVideoProfile>> enumerate_supported_video_profiles(AppGpu &gpu) {
    std::vector<std::unique_ptr<AppVideoProfile>> result{};

    struct ChromaSubsamplingInfo {
        VkVideoChromaSubsamplingFlagsKHR value;
        const char* name;
    };
    const std::vector<ChromaSubsamplingInfo> chroma_subsampling_list = {
        {VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR, "4:2:0"},
        {VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR, "4:2:2"},
        {VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR, "4:4:4"},
        {VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR, "monochrome"}
    };

    struct BitDepthInfo {
        VkVideoComponentBitDepthFlagsKHR value;
        const char* name;
    };
    const std::vector<BitDepthInfo> bit_depth_list = {
        {VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR, "8"},
        {VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR, "10"},
        {VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR, "12"}
    };

    auto find_caps_struct = [](const VkVideoCapabilitiesKHR &capabilities, VkStructureType stype) -> const VkBaseInStructure* {
        auto p = reinterpret_cast<const VkBaseInStructure*>(&capabilities);
        while (p != nullptr) {
            if (p->sType == stype) {
                return p;
            }
            p = p->pNext;
        }
        return nullptr;
    };

    auto base_format = []
        (const ChromaSubsamplingInfo &chroma_subsampling, const BitDepthInfo &luma_bit_depth, const BitDepthInfo &chroma_bit_depth) {
            std::string result{};
            result += " (";
            result += chroma_subsampling.name;
            result += " ";
            result += luma_bit_depth.name;
            if (luma_bit_depth.value != chroma_bit_depth.value) {
                result += ":";
                result += chroma_bit_depth.name;
            }
            result += "-bit)";
            return result;
        };

    auto add_profile = [&](
        const std::string &name,
        const VkVideoProfileInfoKHR &profile_info,
        AppVideoProfile::CreateProfileInfoChainCb create_profile_info_chain,
        AppVideoProfile::CreateCapabilitiesChainCb create_capabilities_chain,
        const AppVideoProfile::CreateFormatPropertiesChainCbList &create_format_properties_chain_list,
        AppVideoProfile::InitProfileCb init_profile) {
            auto profile = std::make_unique<AppVideoProfile>(gpu, gpu.phys_device,
                                                             name, profile_info,
                                                             create_profile_info_chain,
                                                             create_capabilities_chain,
                                                             create_format_properties_chain_list,
                                                             init_profile);
            if (profile->supported) {
                result.push_back(std::move(profile));
            }
        };

    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME)) {
        const std::string codec_name = "H.264 Decode";

        for (auto chroma_subsampling : chroma_subsampling_list) {
            for (auto luma_bit_depth : bit_depth_list) {
                for (auto chroma_bit_depth : bit_depth_list) {
                    if (chroma_subsampling.value == VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR && luma_bit_depth.value != chroma_bit_depth.value) {
                        // Ignore the chroma bit depth dimension for monochrome
                        continue;
                    }

                    std::string profile_base_name = codec_name + base_format(chroma_subsampling, luma_bit_depth, chroma_bit_depth);
                    VkVideoProfileInfoKHR profile_info{
                        VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
                        nullptr,
                        VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR,
                        chroma_subsampling.value,
                        luma_bit_depth.value,
                        chroma_bit_depth.value
                    };

                    auto create_profile_info_chain = [&](const void **ppnext) -> std::unique_ptr<video_profile_info_chain> {
                        auto profile_info_chain = std::make_unique<video_profile_info_chain>();
                        if (profile_info_chain != nullptr) {
                            profile_info_chain->VideoDecodeH264ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR;
                            profile_info_chain->VideoDecodeH264ProfileInfoKHR.pNext = nullptr;
                            *ppnext = &profile_info_chain->VideoDecodeH264ProfileInfoKHR;
                            ppnext = &profile_info_chain->VideoDecodeH264ProfileInfoKHR.pNext;
                        }
                        return profile_info_chain;
                    };

                    auto create_capabilities_chain = [&](void **ppnext) -> std::unique_ptr<video_capabilities_chain> {
                        auto capabilities_chain = std::make_unique<video_capabilities_chain>();
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME)) {
                                capabilities_chain->VideoDecodeCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR;
                                capabilities_chain->VideoDecodeCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoDecodeCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoDecodeCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME)) {
                                capabilities_chain->VideoDecodeH264CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR;
                                capabilities_chain->VideoDecodeH264CapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoDecodeH264CapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoDecodeH264CapabilitiesKHR.pNext;
                            }
                        }
                        return capabilities_chain;
                    };

                    const AppVideoProfile::CreateFormatPropertiesChainCbList create_format_properties_chain_list = {
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Decode Output",
                            VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "DPB",
                            VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                    };

                    add_profile(profile_base_name + " Baseline progressive", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_BASELINE;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR;
                    });
                    add_profile(profile_base_name + " Main progressive", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_MAIN;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR;
                    });
                    add_profile(profile_base_name + " High progressive", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR;
                    });
                    add_profile(profile_base_name + " High 4:4:4 Predictive progressive", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR;
                    });
                    add_profile(profile_base_name + " Baseline interlaced (interleaved lines)", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_BASELINE;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR;
                    });
                    add_profile(profile_base_name + " Main interlaced (interleaved lines)", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_MAIN;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR;
                    });
                    add_profile(profile_base_name + " High interlaced (interleaved lines)", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR;
                    });
                    add_profile(profile_base_name + " High 4:4:4 Predictive interlaced (interleaved lines)", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR;
                    });
                    add_profile(profile_base_name + " Baseline interlaced (separate planes)", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_BASELINE;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR;
                    });
                    add_profile(profile_base_name + " Main interlaced (separate planes)", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_MAIN;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR;
                    });
                    add_profile(profile_base_name + " High interlaced (separate planes)", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR;
                    });
                    add_profile(profile_base_name + " High 4:4:4 Predictive interlaced (separate planes)", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE;
                        profile.profile_info_chain->VideoDecodeH264ProfileInfoKHR.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR;
                    });
                }
            }
        }
    }

    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME)) {
        const std::string codec_name = "H.265 Decode";

        for (auto chroma_subsampling : chroma_subsampling_list) {
            for (auto luma_bit_depth : bit_depth_list) {
                for (auto chroma_bit_depth : bit_depth_list) {
                    if (chroma_subsampling.value == VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR && luma_bit_depth.value != chroma_bit_depth.value) {
                        // Ignore the chroma bit depth dimension for monochrome
                        continue;
                    }

                    std::string profile_base_name = codec_name + base_format(chroma_subsampling, luma_bit_depth, chroma_bit_depth);
                    VkVideoProfileInfoKHR profile_info{
                        VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
                        nullptr,
                        VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR,
                        chroma_subsampling.value,
                        luma_bit_depth.value,
                        chroma_bit_depth.value
                    };

                    auto create_profile_info_chain = [&](const void **ppnext) -> std::unique_ptr<video_profile_info_chain> {
                        auto profile_info_chain = std::make_unique<video_profile_info_chain>();
                        if (profile_info_chain != nullptr) {
                            profile_info_chain->VideoDecodeH265ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR;
                            profile_info_chain->VideoDecodeH265ProfileInfoKHR.pNext = nullptr;
                            *ppnext = &profile_info_chain->VideoDecodeH265ProfileInfoKHR;
                            ppnext = &profile_info_chain->VideoDecodeH265ProfileInfoKHR.pNext;
                        }
                        return profile_info_chain;
                    };

                    auto create_capabilities_chain = [&](void **ppnext) -> std::unique_ptr<video_capabilities_chain> {
                        auto capabilities_chain = std::make_unique<video_capabilities_chain>();
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME)) {
                                capabilities_chain->VideoDecodeCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR;
                                capabilities_chain->VideoDecodeCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoDecodeCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoDecodeCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME)) {
                                capabilities_chain->VideoDecodeH265CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR;
                                capabilities_chain->VideoDecodeH265CapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoDecodeH265CapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoDecodeH265CapabilitiesKHR.pNext;
                            }
                        }
                        return capabilities_chain;
                    };

                    const AppVideoProfile::CreateFormatPropertiesChainCbList create_format_properties_chain_list = {
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Decode Output",
                            VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "DPB",
                            VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                    };

                    add_profile(profile_base_name + " Main", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN;
                    });
                    add_profile(profile_base_name + " Main 10", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN_10;
                    });
                    add_profile(profile_base_name + " Main Still Picture", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE;
                    });
                    add_profile(profile_base_name + " Format range extensions", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_FORMAT_RANGE_EXTENSIONS;
                    });
                    add_profile(profile_base_name + " Screen content coding extensions", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_SCC_EXTENSIONS;
                    });
                }
            }
        }
    }

    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_AV1_EXTENSION_NAME)) {
        const std::string codec_name = "AV1 Decode";

        for (auto chroma_subsampling : chroma_subsampling_list) {
            for (auto luma_bit_depth : bit_depth_list) {
                for (auto chroma_bit_depth : bit_depth_list) {
                    if (chroma_subsampling.value == VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR && luma_bit_depth.value != chroma_bit_depth.value) {
                        // Ignore the chroma bit depth dimension for monochrome
                        continue;
                    }

                    std::string profile_base_name = codec_name + base_format(chroma_subsampling, luma_bit_depth, chroma_bit_depth);
                    VkVideoProfileInfoKHR profile_info{
                        VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
                        nullptr,
                        VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR,
                        chroma_subsampling.value,
                        luma_bit_depth.value,
                        chroma_bit_depth.value
                    };

                    auto create_profile_info_chain = [&](const void **ppnext) -> std::unique_ptr<video_profile_info_chain> {
                        auto profile_info_chain = std::make_unique<video_profile_info_chain>();
                        if (profile_info_chain != nullptr) {
                            profile_info_chain->VideoDecodeAV1ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR;
                            profile_info_chain->VideoDecodeAV1ProfileInfoKHR.pNext = nullptr;
                            *ppnext = &profile_info_chain->VideoDecodeAV1ProfileInfoKHR;
                            ppnext = &profile_info_chain->VideoDecodeAV1ProfileInfoKHR.pNext;
                        }
                        return profile_info_chain;
                    };

                    auto create_capabilities_chain = [&](void **ppnext) -> std::unique_ptr<video_capabilities_chain> {
                        auto capabilities_chain = std::make_unique<video_capabilities_chain>();
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME)) {
                                capabilities_chain->VideoDecodeCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR;
                                capabilities_chain->VideoDecodeCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoDecodeCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoDecodeCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_DECODE_AV1_EXTENSION_NAME)) {
                                capabilities_chain->VideoDecodeAV1CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR;
                                capabilities_chain->VideoDecodeAV1CapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoDecodeAV1CapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoDecodeAV1CapabilitiesKHR.pNext;
                            }
                        }
                        return capabilities_chain;
                    };

                    const AppVideoProfile::CreateFormatPropertiesChainCbList create_format_properties_chain_list = {
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Decode Output",
                            VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "DPB",
                            VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                    };

                    add_profile(profile_base_name + " Main with film grain support", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_MAIN;
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.filmGrainSupport = VK_TRUE;
                    });
                    add_profile(profile_base_name + " High with film grain support", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_HIGH;
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.filmGrainSupport = VK_TRUE;
                    });
                    add_profile(profile_base_name + " Professional with film grain support", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_PROFESSIONAL;
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.filmGrainSupport = VK_TRUE;
                    });
                    add_profile(profile_base_name + " Main without film grain support", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_MAIN;
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.filmGrainSupport = VK_FALSE;
                    });
                    add_profile(profile_base_name + " High without film grain support", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_HIGH;
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.filmGrainSupport = VK_FALSE;
                    });
                    add_profile(profile_base_name + " Professional without film grain support", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_PROFESSIONAL;
                        profile.profile_info_chain->VideoDecodeAV1ProfileInfoKHR.filmGrainSupport = VK_FALSE;
                    });
                }
            }
        }
    }

    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME)) {
        const std::string codec_name = "H.264 Encode";

        for (auto chroma_subsampling : chroma_subsampling_list) {
            for (auto luma_bit_depth : bit_depth_list) {
                for (auto chroma_bit_depth : bit_depth_list) {
                    if (chroma_subsampling.value == VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR && luma_bit_depth.value != chroma_bit_depth.value) {
                        // Ignore the chroma bit depth dimension for monochrome
                        continue;
                    }

                    std::string profile_base_name = codec_name + base_format(chroma_subsampling, luma_bit_depth, chroma_bit_depth);
                    VkVideoProfileInfoKHR profile_info{
                        VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
                        nullptr,
                        VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR,
                        chroma_subsampling.value,
                        luma_bit_depth.value,
                        chroma_bit_depth.value
                    };

                    auto create_profile_info_chain = [&](const void **ppnext) -> std::unique_ptr<video_profile_info_chain> {
                        auto profile_info_chain = std::make_unique<video_profile_info_chain>();
                        if (profile_info_chain != nullptr) {
                            profile_info_chain->VideoEncodeH264ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR;
                            profile_info_chain->VideoEncodeH264ProfileInfoKHR.pNext = nullptr;
                            *ppnext = &profile_info_chain->VideoEncodeH264ProfileInfoKHR;
                            ppnext = &profile_info_chain->VideoEncodeH264ProfileInfoKHR.pNext;
                        }
                        return profile_info_chain;
                    };

                    auto create_capabilities_chain = [&](void **ppnext) -> std::unique_ptr<video_capabilities_chain> {
                        auto capabilities_chain = std::make_unique<video_capabilities_chain>();
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeH264CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeH264CapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeH264CapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeH264CapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeH264QuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeH264QuantizationMapCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeH264QuantizationMapCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeH264QuantizationMapCapabilitiesKHR.pNext;
                            }
                        }
                        return capabilities_chain;
                    };

                    const AppVideoProfile::CreateFormatPropertiesChainCbList create_format_properties_chain_list = {
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Encode Input",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "DPB",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Quantization Delta Map",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                    auto caps = reinterpret_cast<const VkVideoEncodeCapabilitiesKHR*>(find_caps_struct(capabilities, VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR));
                                    if (caps != nullptr) {
                                        supported = supported && ((caps->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) != 0);
                                    } else {
                                        supported = false;
                                    }
                                } else {
                                    supported = false;
                                }
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Emphasis Map",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                    auto caps = reinterpret_cast<const VkVideoEncodeCapabilitiesKHR*>(find_caps_struct(capabilities, VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR));
                                    if (caps != nullptr) {
                                        supported = supported && ((caps->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) != 0);
                                    } else {
                                        supported = false;
                                    }
                                } else {
                                    supported = false;
                                }
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                return format_properties_chain;
                            },
                        },
                    };

                    add_profile(profile_base_name + " Baseline", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_BASELINE;
                    });
                    add_profile(profile_base_name + " Main", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_MAIN;
                    });
                    add_profile(profile_base_name + " High", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH;
                    });
                    add_profile(profile_base_name + " High 4:4:4 Predictive", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH264ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE;
                    });
                }
            }
        }
    }

    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME)) {
        const std::string codec_name = "H.265 Encode";

        for (auto chroma_subsampling : chroma_subsampling_list) {
            for (auto luma_bit_depth : bit_depth_list) {
                for (auto chroma_bit_depth : bit_depth_list) {
                    if (chroma_subsampling.value == VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR && luma_bit_depth.value != chroma_bit_depth.value) {
                        // Ignore the chroma bit depth dimension for monochrome
                        continue;
                    }

                    std::string profile_base_name = codec_name + base_format(chroma_subsampling, luma_bit_depth, chroma_bit_depth);
                    VkVideoProfileInfoKHR profile_info{
                        VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
                        nullptr,
                        VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR,
                        chroma_subsampling.value,
                        luma_bit_depth.value,
                        chroma_bit_depth.value
                    };

                    auto create_profile_info_chain = [&](const void **ppnext) -> std::unique_ptr<video_profile_info_chain> {
                        auto profile_info_chain = std::make_unique<video_profile_info_chain>();
                        if (profile_info_chain != nullptr) {
                            profile_info_chain->VideoEncodeH265ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR;
                            profile_info_chain->VideoEncodeH265ProfileInfoKHR.pNext = nullptr;
                            *ppnext = &profile_info_chain->VideoEncodeH265ProfileInfoKHR;
                            ppnext = &profile_info_chain->VideoEncodeH265ProfileInfoKHR.pNext;
                        }
                        return profile_info_chain;
                    };

                    auto create_capabilities_chain = [&](void **ppnext) -> std::unique_ptr<video_capabilities_chain> {
                        auto capabilities_chain = std::make_unique<video_capabilities_chain>();
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeH265CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeH265CapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeH265CapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeH265CapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeH265QuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeH265QuantizationMapCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeH265QuantizationMapCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeH265QuantizationMapCapabilitiesKHR.pNext;
                            }
                        }
                        return capabilities_chain;
                    };

                    const AppVideoProfile::CreateFormatPropertiesChainCbList create_format_properties_chain_list = {
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Encode Input",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "DPB",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Quantization Delta Map",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                    auto caps = reinterpret_cast<const VkVideoEncodeCapabilitiesKHR*>(find_caps_struct(capabilities, VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR));
                                    if (caps != nullptr) {
                                        supported = supported && ((caps->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) != 0);
                                    } else {
                                        supported = false;
                                    }
                                } else {
                                    supported = false;
                                }
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatH265QuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatH265QuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatH265QuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatH265QuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Emphasis Map",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                    auto caps = reinterpret_cast<const VkVideoEncodeCapabilitiesKHR*>(find_caps_struct(capabilities, VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR));
                                    if (caps != nullptr) {
                                        supported = supported && ((caps->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) != 0);
                                    } else {
                                        supported = false;
                                    }
                                } else {
                                    supported = false;
                                }
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatH265QuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatH265QuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatH265QuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatH265QuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                return format_properties_chain;
                            },
                        },
                    };

                    add_profile(profile_base_name + " Main", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN;
                    });
                    add_profile(profile_base_name + " Main 10", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN_10;
                    });
                    add_profile(profile_base_name + " Main Still Picture", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE;
                    });
                    add_profile(profile_base_name + " Format range extensions", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_FORMAT_RANGE_EXTENSIONS;
                    });
                    add_profile(profile_base_name + " Screen content coding extensions", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeH265ProfileInfoKHR.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_SCC_EXTENSIONS;
                    });
                }
            }
        }
    }

    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_AV1_EXTENSION_NAME)) {
        const std::string codec_name = "AV1 Encode";

        for (auto chroma_subsampling : chroma_subsampling_list) {
            for (auto luma_bit_depth : bit_depth_list) {
                for (auto chroma_bit_depth : bit_depth_list) {
                    if (chroma_subsampling.value == VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR && luma_bit_depth.value != chroma_bit_depth.value) {
                        // Ignore the chroma bit depth dimension for monochrome
                        continue;
                    }

                    std::string profile_base_name = codec_name + base_format(chroma_subsampling, luma_bit_depth, chroma_bit_depth);
                    VkVideoProfileInfoKHR profile_info{
                        VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
                        nullptr,
                        VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR,
                        chroma_subsampling.value,
                        luma_bit_depth.value,
                        chroma_bit_depth.value
                    };

                    auto create_profile_info_chain = [&](const void **ppnext) -> std::unique_ptr<video_profile_info_chain> {
                        auto profile_info_chain = std::make_unique<video_profile_info_chain>();
                        if (profile_info_chain != nullptr) {
                            profile_info_chain->VideoEncodeAV1ProfileInfoKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR;
                            profile_info_chain->VideoEncodeAV1ProfileInfoKHR.pNext = nullptr;
                            *ppnext = &profile_info_chain->VideoEncodeAV1ProfileInfoKHR;
                            ppnext = &profile_info_chain->VideoEncodeAV1ProfileInfoKHR.pNext;
                        }
                        return profile_info_chain;
                    };

                    auto create_capabilities_chain = [&](void **ppnext) -> std::unique_ptr<video_capabilities_chain> {
                        auto capabilities_chain = std::make_unique<video_capabilities_chain>();
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeQuantizationMapCapabilitiesKHR.pNext;
                            }
                        }
                        if (capabilities_chain != nullptr) {
                            if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_AV1_EXTENSION_NAME)) {
                                capabilities_chain->VideoEncodeAV1CapabilitiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR;
                                capabilities_chain->VideoEncodeAV1CapabilitiesKHR.pNext = nullptr;
                                *ppnext = &capabilities_chain->VideoEncodeAV1CapabilitiesKHR;
                                ppnext = &capabilities_chain->VideoEncodeAV1CapabilitiesKHR.pNext;
                            }
                        }
                        return capabilities_chain;
                    };

                    const AppVideoProfile::CreateFormatPropertiesChainCbList create_format_properties_chain_list = {
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Encode Input",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "DPB",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Quantization Delta Map",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                    auto caps = reinterpret_cast<const VkVideoEncodeCapabilitiesKHR*>(find_caps_struct(capabilities, VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR));
                                    if (caps != nullptr) {
                                        supported = supported && ((caps->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) != 0);
                                    } else {
                                        supported = false;
                                    }
                                } else {
                                    supported = false;
                                }
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatAV1QuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatAV1QuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatAV1QuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatAV1QuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                return format_properties_chain;
                            },
                        },
                        AppVideoProfile::CreateFormatPropertiesChainCb {
                            "Emphasis Map",
                            VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                            [&](const VkVideoCapabilitiesKHR &capabilities) -> bool {
                                bool supported = true;
                                if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
                                    auto caps = reinterpret_cast<const VkVideoEncodeCapabilitiesKHR*>(find_caps_struct(capabilities, VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR));
                                    if (caps != nullptr) {
                                        supported = supported && ((caps->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) != 0);
                                    } else {
                                        supported = false;
                                    }
                                } else {
                                    supported = false;
                                }
                                return supported;
                            },
                            [&](void **ppnext) -> std::unique_ptr<video_format_properties_chain> {
                                auto format_properties_chain = std::make_unique<video_format_properties_chain>();
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatQuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                if (format_properties_chain != nullptr) {
                                    if (gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME)) {
                                        format_properties_chain->VideoFormatAV1QuantizationMapPropertiesKHR.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR;
                                        format_properties_chain->VideoFormatAV1QuantizationMapPropertiesKHR.pNext = nullptr;
                                        *ppnext = &format_properties_chain->VideoFormatAV1QuantizationMapPropertiesKHR;
                                        ppnext = &format_properties_chain->VideoFormatAV1QuantizationMapPropertiesKHR.pNext;
                                    }
                                }
                                return format_properties_chain;
                            },
                        },
                    };

                    add_profile(profile_base_name + " Main", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_MAIN;
                    });
                    add_profile(profile_base_name + " High", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_HIGH;
                    });
                    add_profile(profile_base_name + " Professional", profile_info,
                                create_profile_info_chain, create_capabilities_chain,
                                create_format_properties_chain_list,
                                [](AppVideoProfile& profile) {
                        profile.profile_info_chain->VideoEncodeAV1ProfileInfoKHR.stdProfile = STD_VIDEO_AV1_PROFILE_PROFESSIONAL;
                    });
                }
            }
        }
    }
    return result;
}


