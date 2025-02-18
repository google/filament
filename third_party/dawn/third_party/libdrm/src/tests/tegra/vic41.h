/*
 * Copyright © 2018 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef VIC41_H
#define VIC41_H

#include <stdint.h>

#define NVB1B6_VIDEO_COMPOSITOR_SET_APPLICATION_ID 0x00000200
#define NVB1B6_VIDEO_COMPOSITOR_EXECUTE 0x00000300
#define NVB1B6_VIDEO_COMPOSITOR_SET_PICTURE_INDEX 0x00000700
#define NVB1B6_VIDEO_COMPOSITOR_SET_CONTROL_PARAMS 0x00000704
#define NVB1B6_VIDEO_COMPOSITOR_SET_CONFIG_STRUCT_OFFSET 0x00000708
#define NVB1B6_VIDEO_COMPOSITOR_SET_FILTER_STRUCT_OFFSET 0x0000070c
#define NVB1B6_VIDEO_COMPOSITOR_SET_HIST_OFFSET 0x00000714
#define NVB1B6_VIDEO_COMPOSITOR_SET_OUTPUT_SURFACE_LUMA_OFFSET 0x00000720
#define NVB1B6_VIDEO_COMPOSITOR_SET_HISTORY_BUFFER_OFFSET(slot) (0x00000780 + (slot) * 4)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE0_LUMA_OFFSET(slot) (0x00001200 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE0_CHROMA_U_OFFSET(slot) (0x00001204 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE0_CHROMA_V_OFFSET(slot) (0x00001208 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE1_LUMA_OFFSET(slot) (0x0000120c + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE1_CHROMA_U_OFFSET(slot) (0x00001210 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE1_CHROMA_V_OFFSET(slot) (0x00001214 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE2_LUMA_OFFSET(slot) (0x00001218 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE2_CHROMA_U_OFFSET(slot) (0x0000121c + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE2_CHROMA_V_OFFSET(slot) (0x00001220 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE3_LUMA_OFFSET(slot) (0x00001224 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE3_CHROMA_U_OFFSET(slot) (0x00001228 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE3_CHROMA_V_OFFSET(slot) (0x0000122c + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE4_LUMA_OFFSET(slot) (0x00001230 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE4_CHROMA_U_OFFSET(slot) (0x00001234 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE4_CHROMA_V_OFFSET(slot) (0x00001238 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE5_LUMA_OFFSET(slot) (0x0000123c + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE5_CHROMA_U_OFFSET(slot) (0x00001240 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE5_CHROMA_V_OFFSET(slot) (0x00001244 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE6_LUMA_OFFSET(slot) (0x00001248 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE6_CHROMA_U_OFFSET(slot) (0x0000124c + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE6_CHROMA_V_OFFSET(slot) (0x00001250 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE7_LUMA_OFFSET(slot) (0x00001254 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE7_CHROMA_U_OFFSET(slot) (0x00001258 + (slot) * 0x00000060)
#define NVB1B6_VIDEO_COMPOSITOR_SET_SURFACE7_CHROMA_V_OFFSET(slot) (0x0000125c + (slot) * 0x00000060)

typedef struct {
    uint64_t SlotEnable : 1; /* 0 */
    uint64_t DeNoise : 1; /* 1 */
    uint64_t AdvancedDenoise : 1; /* 2 */
    uint64_t CadenceDetect : 1; /* 3 */
    uint64_t MotionMap : 1; /* 4 */
    uint64_t MMapCombine : 1; /* 5 */
    uint64_t IsEven : 1; /* 6 */
    uint64_t ChromaEven : 1; /* 7 */
    uint64_t CurrentFieldEnable : 1; /* 8 */
    uint64_t PrevFieldEnable : 1; /* 9 */
    uint64_t NextFieldEnable : 1; /* 10 */
    uint64_t NextNrFieldEnable : 1; /* 11 */
    uint64_t CurMotionFieldEnable : 1; /* 12 */
    uint64_t PrevMotionFieldEnable : 1; /* 13 */
    uint64_t PpMotionFieldEnable : 1; /* 14 */
    uint64_t CombMotionFieldEnable : 1; /* 15 */
    uint64_t FrameFormat : 4; /* 19..16 */
    uint64_t FilterLengthY : 2; /* 21..20 */
    uint64_t FilterLengthX : 2; /* 23..22 */
    uint64_t Panoramic : 12; /* 35..24 */
    uint64_t ChromaUpLengthY : 2; /* 37..36 */
    uint64_t ChromaUpLengthX : 2; /* 39..38 */
    uint64_t reserved1 : 18; /* 57..40 */
    uint64_t DetailFltClamp : 6; /* 63..58 */
    uint64_t FilterNoise : 10; /* 73..64 */
    uint64_t FilterDetail : 10; /* 83..74 */
    uint64_t ChromaNoise : 10; /* 93..84 */
    uint64_t ChromaDetail : 10; /* 103..94 */
    uint64_t DeinterlaceMode : 4; /* 107..104 */
    uint64_t MotionAccumWeight : 3; /* 110..108 */
    uint64_t NoiseIir : 11; /* 121..111 */
    uint64_t LightLevel : 4; /* 125..122 */
    uint64_t reserved4 : 2; /* 127..126 */
    /* 128 */
    uint64_t SoftClampLow : 10; /* 9..0 */
    uint64_t SoftClampHigh : 10; /* 19..10 */
    uint64_t reserved5 : 12; /* 31..20 */
    uint64_t reserved6 : 2; /* 33..32 */
    uint64_t PlanarAlpha : 8; /* 41..34 */
    uint64_t ConstantAlpha : 1; /* 42 */
    uint64_t StereoInterleave : 3; /* 45..43 */
    uint64_t ClipEnabled : 1; /* 46 */
    uint64_t ClearRectMask : 8; /* 54..47 */
    uint64_t DegammaMode : 2; /* 56..55 */
    uint64_t reserved7 : 1; /* 57 */
    uint64_t DecompressEnable : 1; /* 58 */
    uint64_t DecompressKind : 4; /* 62..59 */
    uint64_t reserved9 : 1; /* 63 */
    uint64_t DecompressCtbCount : 8; /* 71..64 */
    uint64_t DecompressZbcColor : 32; /* 103..72 */
    uint64_t reserved12 : 24; /* 127..104 */
    /* 256 */
    uint64_t SourceRectLeft : 30; /* 29..0 */
    uint64_t reserved14 : 2; /* 31..30 */
    uint64_t SourceRectRight : 30; /* 61..32 */
    uint64_t reserved15 : 2; /* 63..62 */
    uint64_t SourceRectTop : 30; /* 93..64 */
    uint64_t reserved16 : 2; /* 95..94 */
    uint64_t SourceRectBottom : 30; /* 125..96 */
    uint64_t reserved17 : 2; /* 127..126 */
    /* 384 */
    uint64_t DestRectLeft : 14; /* 13..0 */
    uint64_t reserved18 : 2; /* 15..14 */
    uint64_t DestRectRight : 14; /* 29..16 */
    uint64_t reserved19 : 2; /* 31..30 */
    uint64_t DestRectTop : 14; /* 45..32 */
    uint64_t reserved20 : 2; /* 47..46 */
    uint64_t DestRectBottom : 14; /* 61..48 */
    uint64_t reserved21 : 2; /* 63..62 */
    uint64_t reserved22 : 32; /* 95..64 */
    uint64_t reserved23 : 32; /* 127..96 */
} SlotConfig;

typedef struct {
    uint64_t SlotPixelFormat : 7; /* 6..0 */
    uint64_t SlotChromaLocHoriz : 2; /* 8..7 */
    uint64_t SlotChromaLocVert : 2; /* 10..9 */
    uint64_t SlotBlkKind : 4; /* 14..11 */
    uint64_t SlotBlkHeight : 4; /* 18..15 */
    uint64_t SlotCacheWidth : 3; /* 21..19 */
    uint64_t reserved0 : 10; /* 31..22 */
    uint64_t SlotSurfaceWidth : 14; /* 45..32 */
    uint64_t SlotSurfaceHeight : 14; /* 59..46 */
    uint64_t reserved1 : 4; /* 63..60 */
    uint64_t SlotLumaWidth : 14; /* 77..64 */
    uint64_t SlotLumaHeight : 14; /* 91..78 */
    uint64_t reserved2 : 4; /* 95..92 */
    uint64_t SlotChromaWidth : 14; /* 109..96 */
    uint64_t SlotChromaHeight : 14; /* 123..110 */
    uint64_t reserved3 : 4; /* 127..124 */
} SlotSurfaceConfig;

typedef struct {
    uint64_t luma_coeff0 : 20; /* 19..0 */
    uint64_t luma_coeff1 : 20; /* 39..20 */
    uint64_t luma_coeff2 : 20; /* 59..40 */
    uint64_t luma_r_shift : 4; /* 63..60 */
    uint64_t luma_coeff3 : 20; /* 83..64 */
    uint64_t LumaKeyLower : 10; /* 93..84 */
    uint64_t LumaKeyUpper : 10; /* 103..94 */
    uint64_t LumaKeyEnabled : 1; /* 104 */
    uint64_t reserved0 : 2; /* 106..105 */
    uint64_t reserved1 : 21; /* 127..107 */
} LumaKeyStruct;

typedef struct {
    uint64_t matrix_coeff00 : 20; /* 19..0 */
    uint64_t matrix_coeff10 : 20; /* 39..20 */
    uint64_t matrix_coeff20 : 20; /* 59..40 */
    uint64_t matrix_r_shift : 4; /* 63..60 */
    uint64_t matrix_coeff01 : 20; /* 83..64 */
    uint64_t matrix_coeff11 : 20; /* 103..84 */
    uint64_t matrix_coeff21 : 20; /* 123..104 */
    uint64_t reserved0 : 3; /* 126..124 */
    uint64_t matrix_enable : 1; /* 127 */
    /* 128 */
    uint64_t matrix_coeff02 : 20; /* 19..0 */
    uint64_t matrix_coeff12 : 20; /* 39..20 */
    uint64_t matrix_coeff22 : 20; /* 59..40 */
    uint64_t reserved1 : 4; /* 63..60 */
    uint64_t matrix_coeff03 : 20; /* 83..64 */
    uint64_t matrix_coeff13 : 20; /* 103..84 */
    uint64_t matrix_coeff23 : 20; /* 123..104 */
    uint64_t reserved2 : 4; /* 127..124 */
} MatrixStruct;

typedef struct {
    uint64_t ClearRect0Left : 14; /* 13..0 */
    uint64_t reserved0 : 2; /* 15..14 */
    uint64_t ClearRect0Right : 14; /* 29..16 */
    uint64_t reserved1 : 2; /* 31..30 */
    uint64_t ClearRect0Top : 14; /* 45..32 */
    uint64_t reserved2 : 2; /* 47..46 */
    uint64_t ClearRect0Bottom : 14; /* 61..48 */
    uint64_t reserved3 : 2; /* 63..62 */
    uint64_t ClearRect1Left : 14; /* 77..64 */
    uint64_t reserved4 : 2; /* 79..78 */
    uint64_t ClearRect1Right : 14; /* 93..80 */
    uint64_t reserved5 : 2; /* 95..94 */
    uint64_t ClearRect1Top : 14; /* 109..96 */
    uint64_t reserved6 : 2; /* 111..110 */
    uint64_t ClearRect1Bottom : 14; /* 125..112 */
    uint64_t reserved7 : 2; /* 127..126 */
} ClearRectStruct;

typedef struct {
    uint64_t reserved0 : 2; /* 1..0 */
    uint64_t AlphaK1 : 8; /* 9..2 */
    uint64_t reserved1 : 6; /* 17..10 */
    uint64_t AlphaK2 : 8; /* 25..18 */
    uint64_t reserved2 : 6; /* 31..26 */
    uint64_t SrcFactCMatchSelect : 3; /* 34..32 */
    uint64_t reserved3 : 1; /* 35 */
    uint64_t DstFactCMatchSelect : 3; /* 38..36 */
    uint64_t reserved4 : 1; /* 39 */
    uint64_t SrcFactAMatchSelect : 3; /* 42..40 */
    uint64_t reserved5 : 1; /* 43 */
    uint64_t DstFactAMatchSelect : 3; /* 46..44 */
    uint64_t reserved6 : 1; /* 47 */
    uint64_t reserved7 : 4; /* 51..48 */
    uint64_t reserved8 : 4; /* 55..52 */
    uint64_t reserved9 : 4; /* 59..56 */
    uint64_t reserved10 : 4; /* 63..60 */
    uint64_t reserved11 : 2; /* 65..64 */
    uint64_t OverrideR : 10; /* 75..66 */
    uint64_t OverrideG : 10; /* 85..76 */
    uint64_t OverrideB : 10; /* 95..86 */
    uint64_t reserved12 : 2; /* 97..96 */
    uint64_t OverrideA : 8; /* 105..98 */
    uint64_t reserved13 : 2; /* 107..106 */
    uint64_t UseOverrideR : 1; /* 108 */
    uint64_t UseOverrideG : 1; /* 109 */
    uint64_t UseOverrideB : 1; /* 110 */
    uint64_t UseOverrideA : 1; /* 111 */
    uint64_t MaskR : 1; /* 112 */
    uint64_t MaskG : 1; /* 113 */
    uint64_t MaskB : 1; /* 114 */
    uint64_t MaskA : 1; /* 115 */
    uint64_t reserved14 : 12; /* 127..116 */
} BlendingSlotStruct;

typedef struct {
    uint64_t AlphaFillMode : 3; /* 2..0 */
    uint64_t AlphaFillSlot : 3; /* 5..3 */
    uint64_t reserved0 : 2; /* 6..5 */
    uint64_t BackgroundAlpha : 8; /* 15..7 */
    uint64_t BackgroundR : 10; /* 25..16 */
    uint64_t BackgroundG : 10; /* 35..26 */
    uint64_t BackgroundB : 10; /* 45..36 */
    uint64_t RegammaMode : 2; /* 47..46 */
    uint64_t OutputFlipX : 1; /* 48 */
    uint64_t OutputFlipY : 1; /* 49 */
    uint64_t OutputTranspose : 1; /* 50 */
    uint64_t reserved1 : 1; /* 51 */
    uint64_t reserved2 : 12; /* 63..52 */
    uint64_t TargetRectLeft : 14; /* 77..64 */
    uint64_t reserved3 : 2; /* 79..78 */
    uint64_t TargetRectRight : 14; /* 93..80 */
    uint64_t reserved4 : 2; /* 95..94 */
    uint64_t TargetRectTop : 14; /* 109..96 */
    uint64_t reserved5 : 2; /* 111..110 */
    uint64_t TargetRectBottom : 14; /* 125..112 */
    uint64_t reserved6 : 2; /* 127..126 */
} OutputConfig;

typedef struct {
    uint64_t OutPixelFormat : 7; /* 6..0 */
    uint64_t OutChromaLocHoriz : 2; /* 8..7 */
    uint64_t OutChromaLocVert : 2; /* 10..9 */
    uint64_t OutBlkKind : 4; /* 14..11 */
    uint64_t OutBlkHeight : 4; /* 18..15 */
    uint64_t reserved0 : 3; /* 21..19 */
    uint64_t reserved1 : 10; /* 31..22 */
    uint64_t OutSurfaceWidth : 14; /* 45..32 */
    uint64_t OutSurfaceHeight : 14; /* 59..46 */
    uint64_t reserved2 : 4; /* 63..60 */
    uint64_t OutLumaWidth : 14; /* 77..64 */
    uint64_t OutLumaHeight : 14; /* 91..78 */
    uint64_t reserved3 : 4; /* 95..92 */
    uint64_t OutChromaWidth : 14; /* 109..96 */
    uint64_t OutChromaHeight : 14; /* 123..110 */
    uint64_t reserved4 : 4; /* 127..124 */
} OutputSurfaceConfig;

typedef struct {
    uint64_t f00 : 10; /* 9..0 */
    uint64_t f10 : 10; /* 19..10 */
    uint64_t f20 : 10; /* 29..20 */
    uint64_t reserved0 : 2; /* 31..30 */
    uint64_t f01 : 10; /* 41..32 */
    uint64_t f11 : 10; /* 51..42 */
    uint64_t f21 : 10; /* 61..52 */
    uint64_t reserved1 : 2; /* 63..62 */
    uint64_t f02 : 10; /* 73..64 */
    uint64_t f12 : 10; /* 83..74 */
    uint64_t f22 : 10; /* 93..84 */
    uint64_t reserved2 : 2; /* 95..94 */
    uint64_t f03 : 10; /* 105..96 */
    uint64_t f13 : 10; /* 115..106 */
    uint64_t f23 : 10; /* 125..116 */
    uint64_t reserved3 : 2; /* 127..126 */
} FilterCoeffStruct;

typedef struct {
    uint64_t DownsampleHoriz : 11; /* 10..0 */
    uint64_t reserved0 : 5; /* 15..11 */
    uint64_t DownsampleVert : 11; /* 26..16 */
    uint64_t reserved1 : 5; /* 31..27 */
    uint64_t reserved2 : 32; /* 63..32 */
    uint64_t reserved3 : 32; /* 95..64 */
    uint64_t reserved4 : 32; /* 127..96 */
} PipeConfig;

typedef struct {
    uint64_t OldCadence : 32; /* 31..0 */
    uint64_t OldDiff : 32; /* 63..32 */
    uint64_t OldWeave : 32; /* 95..64 */
    uint64_t OlderWeave : 32; /* 127..96 */
} SlotHistoryBuffer;

typedef struct {
    uint64_t crc0 : 32; /* 31..0 */
    uint64_t crc1 : 32; /* 63..32 */
    uint64_t crc2 : 32; /* 95..64 */
    uint64_t crc3 : 32; /* 127..96 */
} PartitionCrcStruct;

typedef struct {
    uint64_t crc0 : 32; /* 31..0 */
    uint64_t crc1 : 32; /* 63..32 */
} SlotCrcStruct;

typedef struct {
    uint64_t ErrorStatus : 32; /* 31..0 */
    uint64_t CycleCount : 32; /* 63..32 */
    uint64_t reserved0 : 32; /* 95..64 */
    uint64_t reserved1 : 32; /* 127..96 */
} StatusStruct;

typedef struct {
    SlotConfig slotConfig;
    SlotSurfaceConfig slotSurfaceConfig;
    LumaKeyStruct lumaKeyStruct;
    MatrixStruct colorMatrixStruct;
    MatrixStruct gamutMatrixStruct;
    BlendingSlotStruct blendingSlotStruct;
} SlotStruct;

typedef struct {
    FilterCoeffStruct filterCoeffStruct[520];
} FilterStruct;

typedef struct {
    PipeConfig pipeConfig;
    OutputConfig outputConfig;
    OutputSurfaceConfig outputSurfaceConfig;
    MatrixStruct outColorMatrixStruct;
    ClearRectStruct clearRectStruct[4];
    SlotStruct slotStruct[16];
} ConfigStruct;

typedef struct {
    PartitionCrcStruct partitionCrcStruct[4];
} InterfaceCrcStruct;

typedef struct {
    SlotCrcStruct slotCrcStruct[16];
} InputCrcStruct;

#endif
