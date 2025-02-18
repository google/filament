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

#ifndef VIC30_H
#define VIC30_H

#include <stdint.h>

#define NVA0B6_VIDEO_COMPOSITOR_SET_APPLICATION_ID 0x200
#define NVA0B6_VIDEO_COMPOSITOR_EXECUTE 0x300
#define NVA0B6_VIDEO_COMPOSITOR_EXECUTE_AWAKEN (1 << 8)
#define NVA0B6_VIDEO_COMPOSITOR_SET_SURFACE0_SLOT0_LUMA_OFFSET 0x400
#define NVA0B6_VIDEO_COMPOSITOR_SET_SURFACE0_SLOT0_CHROMA_U_OFFSET 0x404
#define NVA0B6_VIDEO_COMPOSITOR_SET_SURFACE0_SLOT0_CHROMA_V_OFFSET 0x408
#define NVA0B6_VIDEO_COMPOSITOR_SET_CONTROL_PARAMS 0x700
#define NVA0B6_VIDEO_COMPOSITOR_SET_CONFIG_STRUCT_OFFSET 0x720
#define NVA0B6_VIDEO_COMPOSITOR_SET_PALETTE_OFFSET 0x724
#define NVA0B6_VIDEO_COMPOSITOR_SET_HIST_OFFSET 0x728
#define NVA0B6_VIDEO_COMPOSITOR_SET_OUTPUT_SURFACE_LUMA_OFFSET 0x730
#define NVA0B6_VIDEO_COMPOSITOR_SET_OUTPUT_SURFACE_CHROMA_U_OFFSET 0x734
#define NVA0B6_VIDEO_COMPOSITOR_SET_OUTPUT_SURFACE_CHROMA_V_OFFSET 0x738

#define VIC_PIXEL_FORMAT_L8 1
#define VIC_PIXEL_FORMAT_R8 4
#define VIC_PIXEL_FORMAT_A8R8G8B8 32
#define VIC_PIXEL_FORMAT_R8G8B8A8 34
#define VIC_PIXEL_FORMAT_Y8_U8V8_N420 67
#define VIC_PIXEL_FORMAT_Y8_V8U8_N420 68

#define VIC_BLK_KIND_PITCH 0
#define VIC_BLK_KIND_GENERIC_16Bx2 1

typedef struct {
    uint64_t DeNoise0 : 1; /* 0 */
    uint64_t CadenceDetect0 : 1; /* 1 */
    uint64_t MotionMap0 : 1; /* 2 */
    uint64_t MedianFilter0 : 1; /* 3 */
    uint64_t DeNoise1 : 1; /* 4 */
    uint64_t CadenceDetect1 : 1; /* 5 */
    uint64_t MotionMap1 : 1; /* 6 */
    uint64_t MedianFilter1 : 1; /* 7 */
    uint64_t DeNoise2 : 1; /* 8 */
    uint64_t CadenceDetect2 : 1; /* 9 */
    uint64_t MotionMap2 : 1; /* 10 */
    uint64_t MedianFilter2 : 1; /* 11 */
    uint64_t DeNoise3 : 1; /* 12 */
    uint64_t CadenceDetect3 : 1; /* 13 */
    uint64_t MotionMap3 : 1; /* 14 */
    uint64_t MedianFilter3 : 1; /* 15 */
    uint64_t DeNoise4 : 1; /* 16 */
    uint64_t CadenceDetect4 : 1; /* 17 */
    uint64_t MotionMap4 : 1; /* 18 */
    uint64_t MedianFilter4 : 1; /* 19 */
    uint64_t IsEven0 : 1; /* 20 */
    uint64_t IsEven1 : 1; /* 21 */
    uint64_t IsEven2 : 1; /* 22 */
    uint64_t IsEven3 : 1; /* 23 */
    uint64_t IsEven4 : 1; /* 24 */
    uint64_t MMapCombine0 : 1; /* 25 */
    uint64_t MMapCombine1 : 1; /* 26 */
    uint64_t MMapCombine2 : 1; /* 27 */
    uint64_t MMapCombine3 : 1; /* 28 */
    uint64_t MMapCombine4 : 1; /* 29 */
    uint64_t reserved0 : 2; /* 31..30 */
    uint64_t PixelFormat0 : 7; /* 38..32 */
    uint64_t reserved1 : 1; /* 39 */
    uint64_t PixelFormat1 : 7; /* 46..40 */
    uint64_t reserved2 : 1; /* 47 */
    uint64_t PixelFormat2 : 7; /* 54..48 */
    uint64_t reserved3 : 1; /* 55 */
    uint64_t PixelFormat3 : 7; /* 62..56 */
    uint64_t reserved4 : 1; /* 63 */
    uint64_t PixelFormat4 : 7; /* 70..64 */
    uint64_t reserved5 : 1; /* 71 */
    uint64_t reserved6 : 24; /* 95..72 */
    uint64_t PPMotion0 : 1; /* 96 */
    uint64_t PPMotion1 : 1; /* 97 */
    uint64_t PPMotion2 : 1; /* 98 */
    uint64_t PPMotion3 : 1; /* 99 */
    uint64_t PPMotion4 : 1; /* 100 */
    uint64_t reserved7 : 3; /* 103..101 */
    uint64_t ChromaEven0 : 1; /* 104 */
    uint64_t ChromaEven1 : 1; /* 105 */
    uint64_t ChromaEven2 : 1; /* 106 */
    uint64_t ChromaEven3 : 1; /* 107 */
    uint64_t ChromaEven4 : 1; /* 108 */
    uint64_t reserved8 : 3; /* 111..109 */
    uint64_t AdvancedDenoise0 : 1; /* 112 */
    uint64_t AdvancedDenoise1 : 1; /* 113 */
    uint64_t AdvancedDenoise2 : 1; /* 114 */
    uint64_t AdvancedDenoise3 : 1; /* 115 */
    uint64_t AdvancedDenoise4 : 1; /* 116 */
    uint64_t reserved9 : 3; /* 119..117 */
    uint64_t reserved10 : 8; /* 127..120 */
} SurfaceCache0Struct;

typedef struct {
    uint64_t ClearRectMask0 : 8; /* 7..0 */
    uint64_t ClearRectMask1 : 8; /* 15..8 */
    uint64_t ClearRectMask2 : 8; /* 23..16 */
    uint64_t ClearRectMask3 : 8; /* 31..24 */
    uint64_t ClearRectMask4 : 8; /* 39..32 */
    uint64_t reserved0 : 22; /* 61..40 */
    uint64_t OutputFlipX : 1; /* 62 */
    uint64_t OutputFlipY : 1; /* 63 */
    uint64_t TargetRectLeft : 14; /* 77..64 */
    uint64_t reserved1 : 2; /* 79..78 */
    uint64_t TargetRectRight : 14; /* 93..80 */
    uint64_t reserved2 : 2; /* 95..94 */
    uint64_t TargetRectTop : 14; /* 109..96 */
    uint64_t reserved3 : 2; /* 111..110 */
    uint64_t TargetRectBottom : 14; /* 125..112 */
    uint64_t reserved4 : 2; /* 127..126 */
} SurfaceList0Struct;

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
} SurfaceListClearRectStruct;

typedef struct {
    uint64_t Enable : 1; /* 0 */
    uint64_t FrameFormat : 4; /* 4..1 */
    uint64_t PixelFormat : 7; /* 11..5 */
    uint64_t reserved0 : 2; /* 13..12 */
    uint64_t ChromaLocHoriz : 2; /* 15..14 */
    uint64_t ChromaLocVert : 2; /* 17..16 */
    uint64_t Panoramic : 12; /* 29..18 */
    uint64_t reserved1 : 4; /* 33..30 */
    uint64_t SurfaceWidth : 14; /* 47..34 */
    uint64_t reserved2 : 1; /* 48 */
    uint64_t SurfaceHeight : 14; /* 62..49 */
    uint64_t reserved3 : 1; /* 63 */
    uint64_t LumaWidth : 14; /* 77..64 */
    uint64_t reserved4 : 1; /* 78 */
    uint64_t LumaHeight : 14; /* 92..79 */
    uint64_t reserved5 : 1; /* 93 */
    uint64_t ChromaWidth : 14; /* 107..94 */
    uint64_t reserved6 : 1; /* 108 */
    uint64_t ChromaHeight : 14; /* 122..109 */
    uint64_t reserved7 : 1; /* 123 */
    uint64_t CacheWidth : 3; /* 126..124 */
    uint64_t reserved8 : 1; /* 127 */
    /* 128 */
    uint64_t FilterLengthY : 2; /* 1..0 */
    uint64_t FilterLengthX : 2; /* 3..2 */
    uint64_t DetailFltClamp : 6; /* 9..4 */
    uint64_t reserved9 : 2; /* 11..10 */
    uint64_t LightLevel : 4; /* 15..12 */
    uint64_t reserved10 : 4; /* 19..16 */
    uint64_t reserved11 : 8; /* 27..20 */
    uint64_t reserved12 : 32; /* 59..28 */
    uint64_t BlkKind : 4; /* 63..60 */
    uint64_t DestRectLeft : 14; /* 77..64 */
    uint64_t reserved13 : 1; /* 78 */
    uint64_t DestRectRight : 14; /* 92..79 */
    uint64_t reserved14 : 1; /* 93 */
    uint64_t DestRectTop : 14; /* 107..94 */
    uint64_t reserved15 : 1; /* 108 */
    uint64_t DestRectBottom : 14; /* 122..109 */
    uint64_t reserved16 : 1; /* 123 */
    uint64_t BlkHeight : 4; /* 127..124 */
    /* 256 */
    uint64_t SourceRectLeft : 30; /* 29..0 */
    uint64_t reserved17 : 2; /* 31..30 */
    uint64_t SourceRectRight : 30; /* 61..32 */
    uint64_t reserved18 : 2; /* 63..62 */
    uint64_t SourceRectTop : 30; /* 93..64 */
    uint64_t reserved19 : 2; /* 95..94 */
    uint64_t SourceRectBottom : 30; /* 125..96 */
    uint64_t reserved20 : 2; /* 127..126 */
} SurfaceListSurfaceStruct;

typedef struct {
    uint64_t l0 : 20; /* 19..0 */
    uint64_t l1 : 20; /* 39..20 */
    uint64_t l2 : 20; /* 59..40 */
    uint64_t r_shift : 4; /* 63..60 */
    uint64_t l3 : 20; /* 83..64 */
    uint64_t PlanarAlpha : 10; /* 93..84 */
    uint64_t ConstantAlpha : 1; /* 94 */
    uint64_t ClipEnabled : 1; /* 95 */
    uint64_t LumaKeyLower : 10; /* 105..96 */
    uint64_t reserved6 : 3; /* 108..106 */
    uint64_t StereoInterleave : 3; /* 111..109 */
    uint64_t LumaKeyUpper : 10; /* 121..112 */
    uint64_t reserved7 : 2; /* 123..122 */
    uint64_t reserved8 : 1; /* 124 */
    uint64_t LumaKeyEnabled : 1; /* 125 */
    uint64_t reserved9 : 2; /* 127..126 */
} ColorConversionLumaAlphaStruct;

typedef struct {
    uint64_t c00 : 20; /* 19..0 */
    uint64_t c10 : 20; /* 39..20 */
    uint64_t c20 : 20; /* 59..40 */
    uint64_t r_shift : 4; /* 63..60 */
    uint64_t c01 : 20; /* 83..64 */
    uint64_t c11 : 20; /* 103..84 */
    uint64_t c21 : 20; /* 123..104 */
    uint64_t reserved0 : 4; /* 127..124 */
    /* 128 */
    uint64_t c02 : 20; /* 19..0 */
    uint64_t c12 : 20; /* 39..20 */
    uint64_t c22 : 20; /* 59..40 */
    uint64_t reserved1 : 4; /* 63..60 */
    uint64_t c03 : 20; /* 83..64 */
    uint64_t c13 : 20; /* 103..84 */
    uint64_t c23 : 20; /* 123..104 */
    uint64_t reserved2 : 4; /* 127..124 */
} ColorConversionMatrixStruct;

typedef struct {
    uint64_t low : 10; /* 9..0 */
    uint64_t reserved0 : 6; /* 15..10 */
    uint64_t high : 10; /* 25..16 */
    uint64_t reserved1 : 6; /* 31..26 */
    uint64_t reserved2 : 32; /* 63..32 */
    uint64_t reserved3 : 32; /* 95..64 */
    uint64_t reserved4 : 32; /* 127..96 */
} ColorConversionClampStruct;

typedef struct {
    uint64_t PixelFormat : 7; /* 6..0 */
    uint64_t reserved0 : 1; /* 7 */
    uint64_t AlphaFillMode : 3; /* 10..8 */
    uint64_t AlphaFillSlot : 3; /* 13..11 */
    uint64_t BackgroundAlpha : 10; /* 23..14 */
    uint64_t BackgroundR : 10; /* 33..24 */
    uint64_t BackgroundG : 10; /* 43..34 */
    uint64_t BackgroundB : 10; /* 53..44 */
    uint64_t ChromaLocHoriz : 2; /* 55..54 */
    uint64_t ChromaLocVert : 2; /* 57..56 */
    uint64_t reserved1 : 6; /* 63..58 */
    uint64_t LumaWidth : 14; /* 77..64 */
    uint64_t reserved2 : 2; /* 79..78 */
    uint64_t LumaHeight : 14; /* 93..80 */
    uint64_t reserved3 : 2; /* 95..94 */
    uint64_t ChromaWidth : 14; /* 109..96 */
    uint64_t reserved4 : 2; /* 111..110 */
    uint64_t ChromaHeight : 14; /* 125..112 */
    uint64_t reserved5 : 2; /* 127..126 */
    /* 128 */
    uint64_t TargetRectLeft : 14; /* 13..0 */
    uint64_t reserved6 : 2; /* 15..14 */
    uint64_t TargetRectRight : 14; /* 29..16 */
    uint64_t reserved7 : 2; /* 31..30 */
    uint64_t TargetRectTop : 14; /* 45..32 */
    uint64_t reserved8 : 2; /* 47..46 */
    uint64_t TargetRectBottom : 14; /* 61..48 */
    uint64_t reserved9 : 2; /* 63..62 */
    uint64_t SurfaceWidth : 14; /* 77..64 */
    uint64_t reserved10 : 2; /* 79..78 */
    uint64_t SurfaceHeight : 14; /* 93..80 */
    uint64_t reserved11 : 2; /* 95..94 */
    uint64_t BlkKind : 4; /* 99..96 */
    uint64_t BlkHeight : 4; /* 103..100 */
    uint64_t OutputFlipX : 1; /* 104 */
    uint64_t OutputFlipY : 1; /* 105 */
    uint64_t OutputTranspose : 1; /* 106 */
    uint64_t reserved12 : 21; /* 127..107 */
} Blending0Struct;

typedef struct {
    uint64_t AlphaK1 : 10; /* 9..0 */
    uint64_t reserved0 : 6; /* 15..10 */
    uint64_t AlphaK2 : 10; /* 25..16 */
    uint64_t reserved1 : 6; /* 31..26 */
    uint64_t SrcFactCMatchSelect : 3; /* 34..32 */
    uint64_t reserved2 : 1; /* 35 */
    uint64_t DstFactCMatchSelect : 3; /* 38..36 */
    uint64_t reserved3 : 1; /* 39 */
    uint64_t SrcFactAMatchSelect : 3; /* 42..40 */
    uint64_t reserved4 : 1; /* 43 */
    uint64_t DstFactAMatchSelect : 3; /* 46..44 */
    uint64_t reserved5 : 1; /* 47 */
    uint64_t reserved6 : 4; /* 51..48 */
    uint64_t reserved7 : 4; /* 55..52 */
    uint64_t reserved8 : 4; /* 59..56 */
    uint64_t reserved9 : 4; /* 63..60 */
    uint64_t reserved10 : 2; /* 65..64 */
    uint64_t OverrideR : 10; /* 75..66 */
    uint64_t OverrideG : 10; /* 85..76 */
    uint64_t OverrideB : 10; /* 95..86 */
    uint64_t OverrideA : 10; /* 105..96 */
    uint64_t reserved11 : 2; /* 107..106 */
    uint64_t UseOverrideR : 1; /* 108 */
    uint64_t UseOverrideG : 1; /* 109 */
    uint64_t UseOverrideB : 1; /* 110 */
    uint64_t UseOverrideA : 1; /* 111 */
    uint64_t MaskR : 1; /* 112 */
    uint64_t MaskG : 1; /* 113 */
    uint64_t MaskB : 1; /* 114 */
    uint64_t MaskA : 1; /* 115 */
    uint64_t reserved12 : 12; /* 127..116 */
} BlendingSurfaceStruct;

typedef struct {
    uint64_t TargetRectLeft : 14; /* 13..0 */
    uint64_t reserved0 : 2; /* 15..14 */
    uint64_t TargetRectRight : 14; /* 29..16 */
    uint64_t reserved1 : 2; /* 31..30 */
    uint64_t TargetRectTop : 14; /* 45..32 */
    uint64_t reserved2 : 2; /* 47..46 */
    uint64_t TargetRectBottom : 14; /* 61..48 */
    uint64_t reserved3 : 2; /* 63..62 */
    uint64_t Enable0 : 8; /* 71..64 */
    uint64_t Enable1 : 8; /* 79..72 */
    uint64_t Enable2 : 8; /* 87..80 */
    uint64_t Enable3 : 8; /* 95..88 */
    uint64_t Enable4 : 8; /* 103..96 */
    uint64_t DownsampleHoriz : 11; /* 114..104 */
    uint64_t reserved4 : 1; /* 115 */
    uint64_t DownsampleVert : 11; /* 126..116 */
    uint64_t reserved5 : 1; /* 127 */
    /* 128 */
    uint64_t FilterNoise0 : 10; /* 9..0 */
    uint64_t FilterDetail0 : 10; /* 19..10 */
    uint64_t FilterNoise1 : 10; /* 29..20 */
    uint64_t reserved6 : 2; /* 31..30 */
    uint64_t FilterDetail1 : 10; /* 41..32 */
    uint64_t FilterNoise2 : 10; /* 51..42 */
    uint64_t FilterDetail2 : 10; /* 61..52 */
    uint64_t reserved7 : 2; /* 63..62 */
    uint64_t FilterNoise3 : 10; /* 73..64 */
    uint64_t FilterDetail3 : 10; /* 83..74 */
    uint64_t FilterNoise4 : 10; /* 93..84 */
    uint64_t reserved8 : 2; /* 95..94 */
    uint64_t FilterDetail4 : 10; /* 105..96 */
    uint64_t reserved9 : 22; /* 127..106 */
    /* 256 */
    uint64_t ChromaNoise0 : 10; /* 9..0 */
    uint64_t ChromaDetail0 : 10; /* 19..10 */
    uint64_t ChromaNoise1 : 10; /* 29..20 */
    uint64_t reserved10 : 2; /* 31..30 */
    uint64_t ChromaDetail1 : 10; /* 41..32 */
    uint64_t ChromaNoise2 : 10; /* 51..42 */
    uint64_t ChromaDetail2 : 10; /* 61..52 */
    uint64_t reserved11 : 2; /* 63..62 */
    uint64_t ChromaNoise3 : 10; /* 73..64 */
    uint64_t ChromaDetail3 : 10; /* 83..74 */
    uint64_t ChromaNoise4 : 10; /* 93..84 */
    uint64_t reserved12 : 2; /* 95..94 */
    uint64_t ChromaDetail4 : 10; /* 105..96 */
    uint64_t reserved13 : 22; /* 127..106 */
    /* 384 */
    uint64_t Mode0 : 4; /* 3..0 */
    uint64_t AccumWeight0 : 3; /* 6..4 */
    uint64_t Iir0 : 11; /* 17..7 */
    uint64_t reserved14 : 2; /* 19..18 */
    uint64_t Mode1 : 4; /* 23..20 */
    uint64_t AccumWeight1 : 3; /* 26..24 */
    uint64_t Iir1 : 11; /* 37..27 */
    uint64_t reserved15 : 2; /* 39..38 */
    uint64_t Mode2 : 4; /* 43..40 */
    uint64_t AccumWeight2 : 3; /* 46..44 */
    uint64_t Iir2 : 11; /* 57..47 */
    uint64_t reserved16 : 6; /* 63..58 */
    uint64_t Mode3 : 4; /* 67..64 */
    uint64_t AccumWeight3 : 3; /* 70..68 */
    uint64_t Iir3 : 11; /* 81..71 */
    uint64_t reserved17 : 2; /* 83..82 */
    uint64_t Mode4 : 4; /* 87..84 */
    uint64_t AccumWeight4 : 3; /* 90..88 */
    uint64_t Iir4 : 11; /* 101..91 */
    uint64_t reserved18 : 8; /* 109..102 */
    uint64_t OutputFlipX : 1; /* 110 */
    uint64_t OutputFlipY : 1; /* 111 */
    uint64_t reserved19 : 10; /* 121..112 */
    uint64_t reserved20 : 6; /* 127..122 */
} FetchControl0Struct;

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
} FetchControlCoeffStruct;

typedef struct {
    SurfaceCache0Struct surfaceCache0Struct;
    SurfaceList0Struct surfaceList0Struct;
    SurfaceListClearRectStruct surfaceListClearRectStruct[4];
    SurfaceListSurfaceStruct surfaceListSurfaceStruct[5];
    ColorConversionLumaAlphaStruct colorConversionLumaAlphaStruct[5];
    ColorConversionMatrixStruct colorConversionMatrixStruct[5];
    ColorConversionClampStruct colorConversionClampStruct[5];
    Blending0Struct blending0Struct;
    BlendingSurfaceStruct blendingSurfaceStruct[5];
    FetchControl0Struct fetchControl0Struct;
    FetchControlCoeffStruct fetchControlCoeffStruct[520];
} ConfigStruct;

#endif
