// File: basisu_wasm_api_common.h
#pragma once
#include "stdint.h"

#if defined(__wasm__)
	#if defined(__cplusplus)
		#define BU_WASM_EXPORT(name) __attribute__((export_name(name))) extern "C"
	#else
		#define BU_WASM_EXPORT(name) __attribute__((export_name(name)))
	#endif
#elif defined(__cplusplus)
	#define BU_WASM_EXPORT(name) extern "C"
#else
	#define BU_WASM_EXPORT(name)
#endif

// wasm_bool_t is an alias for uint32_t
typedef uint32_t wasm_bool_t;

// Compression constants

#define BU_QUALITY_MIN 0
#define BU_QUALITY_MAX 100

#define BU_EFFORT_MIN 0
#define BU_EFFORT_MAX 10
#define BU_EFFORT_SUPER_FAST = 0
#define BU_EFFORT_FAST = 2
#define BU_EFFORT_NORMAL = 5
#define BU_EFFORT_DEFAULT = 2
#define BU_EFFORT_SLOW = 8
#define BU_EFFORT_VERY_SLOW = 10

#define BU_COMP_FLAGS_NONE  					(0)
#define BU_COMP_FLAGS_USE_OPENCL       			(1 << 8 )
#define BU_COMP_FLAGS_THREADED         			(1 << 9 )
#define BU_COMP_FLAGS_DEBUG_OUTPUT     			(1 << 10)
#define BU_COMP_FLAGS_KTX2_OUTPUT      			(1 << 11)
#define BU_COMP_FLAGS_KTX2_UASTC_ZSTD  			(1 << 12)
#define BU_COMP_FLAGS_SRGB             			(1 << 13)
#define BU_COMP_FLAGS_GEN_MIPS_CLAMP   			(1 << 14)
#define BU_COMP_FLAGS_GEN_MIPS_WRAP    			(1 << 15)
#define BU_COMP_FLAGS_Y_FLIP           			(1 << 16)
#define BU_COMP_FLAGS_PRINT_STATS      			(1 << 18)
#define BU_COMP_FLAGS_PRINT_STATUS     			(1 << 19)
#define BU_COMP_FLAGS_DEBUG_IMAGES     			(1 << 20)
#define BU_COMP_FLAGS_REC2020          			(1 << 21)
#define BU_COMP_FLAGS_VALIDATE_OUTPUT  			(1 << 22)

#define BU_COMP_FLAGS_XUASTC_LDR_FULL_ARITH  	(0)
#define BU_COMP_FLAGS_XUASTC_LDR_HYBRID       	(1 << 23)
#define BU_COMP_FLAGS_XUASTC_LDR_FULL_ZSTD    	(2 << 23)
#define BU_COMP_FLAGS_XUASTC_LDR_SYNTAX_SHIFT  	(23)
#define BU_COMP_FLAGS_XUASTC_LDR_SYNTAX_MASK   	(3)

#define BU_COMP_FLAGS_TEXTURE_TYPE_2D				(0 << 25)
#define BU_COMP_FLAGS_TEXTURE_TYPE_2D_ARRAY			(1 << 25)
#define BU_COMP_FLAGS_TEXTURE_TYPE_CUBEMAP_ARRAY	(2 << 25)
#define BU_COMP_FLAGS_TEXTURE_TYPE_VIDEO_FRAMES		(3 << 25)
#define BU_COMP_FLAGS_TEXTURE_TYPE_SHIFT			(25)
#define BU_COMP_FLAGS_TEXTURE_TYPE_MASK				(3)

#define BU_COMP_FLAGS_VERBOSE (BU_COMP_FLAGS_DEBUG_OUTPUT | BU_COMP_FLAGS_PRINT_STATS | BU_COMP_FLAGS_PRINT_STATUS)

// basist::basis_tex_format: the supported .ktx2 (and .basis) file format types
#define BTF_ETC1S 0
#define BTF_UASTC_LDR_4X4 1
#define BTF_UASTC_HDR_4X4 2
#define BTF_ASTC_HDR_6X6 3
#define BTF_UASTC_HDR_6X6 4
#define BTF_XUASTC_LDR_4X4 5
#define BTF_XUASTC_LDR_5X4 6
#define BTF_XUASTC_LDR_5X5 7
#define BTF_XUASTC_LDR_6X5 8
#define BTF_XUASTC_LDR_6X6 9
#define BTF_XUASTC_LDR_8X5 10
#define BTF_XUASTC_LDR_8X6 11
#define BTF_XUASTC_LDR_10X5 12
#define BTF_XUASTC_LDR_10X6 13
#define BTF_XUASTC_LDR_8X8 14
#define BTF_XUASTC_LDR_10X8 15
#define BTF_XUASTC_LDR_10X10 16
#define BTF_XUASTC_LDR_12X10 17
#define BTF_XUASTC_LDR_12X12 18
#define BTF_ASTC_LDR_4X4 19
#define BTF_ASTC_LDR_5X4 20
#define BTF_ASTC_LDR_5X5 21
#define BTF_ASTC_LDR_6X5 22
#define BTF_ASTC_LDR_6X6 23
#define BTF_ASTC_LDR_8X5 24
#define BTF_ASTC_LDR_8X6 25
#define BTF_ASTC_LDR_10X5 26
#define BTF_ASTC_LDR_10X6 27
#define BTF_ASTC_LDR_8X8 28
#define BTF_ASTC_LDR_10X8 29
#define BTF_ASTC_LDR_10X10 30
#define BTF_ASTC_LDR_12X10 31
#define BTF_ASTC_LDR_12X12 32
#define BTF_TOTAL_FORMATS 33

// Transcoding constants

// basist::transcoder_texture_format: the supported transcode GPU texture formats
#define TF_ETC1_RGB 0
#define TF_ETC2_RGBA 1
#define TF_BC1_RGB 2
#define TF_BC3_RGBA 3
#define TF_BC4_R 4
#define TF_BC5_RG 5
#define TF_BC7_RGBA 6
#define TF_PVRTC1_4_RGB 8
#define TF_PVRTC1_4_RGBA 9
#define TF_ASTC_LDR_4X4_RGBA 10
#define TF_ATC_RGB 11
#define TF_ATC_RGBA 12
#define TF_FXT1_RGB 17
#define TF_PVRTC2_4_RGB 18
#define TF_PVRTC2_4_RGBA 19
#define TF_ETC2_EAC_R11 20
#define TF_ETC2_EAC_RG11 21
#define TF_BC6H 22
#define TF_ASTC_HDR_4X4_RGBA 23
#define TF_RGBA32 13
#define TF_RGB565 14
#define TF_BGR565 15
#define TF_RGBA4444 16
#define TF_RGB_HALF 24
#define TF_RGBA_HALF 25
#define TF_RGB_9E5 26
#define TF_ASTC_HDR_6X6_RGBA 27
#define TF_ASTC_LDR_5X4_RGBA 28
#define TF_ASTC_LDR_5X5_RGBA 29
#define TF_ASTC_LDR_6X5_RGBA 30
#define TF_ASTC_LDR_6X6_RGBA 31
#define TF_ASTC_LDR_8X5_RGBA 32
#define TF_ASTC_LDR_8X6_RGBA 33
#define TF_ASTC_LDR_10X5_RGBA 34
#define TF_ASTC_LDR_10X6_RGBA 35
#define TF_ASTC_LDR_8X8_RGBA 36
#define TF_ASTC_LDR_10X8_RGBA 37
#define TF_ASTC_LDR_10X10_RGBA 38
#define TF_ASTC_LDR_12X10_RGBA 39
#define TF_ASTC_LDR_12X12_RGBA 40
#define TF_TOTAL_TEXTURE_FORMATS 41

// basist::basisu_decode_flags: Transcode decode flags (bt_ktx2_transcode_image_level decode_flags parameter, logically OR'd)
#define DECODE_FLAGS_PVRTC_DECODE_TO_NEXT_POW2 2
#define DECODE_FLAGS_TRANSCODE_ALPHA_DATA_TO_OPAQUE_FORMATS 4
#define DECODE_FLAGS_BC1_FORBID_THREE_COLOR_BLOCKS 8
#define DECODE_FLAGS_OUTPUT_HAS_ALPHA_INDICES 16
#define DECODE_FLAGS_HIGH_QUALITY 32
#define DECODE_FLAGS_NO_ETC1S_CHROMA_FILTERING 64
#define DECODE_FLAGS_NO_DEBLOCK_FILTERING 128
#define DECODE_FLAGS_STRONGER_DEBLOCK_FILTERING 256
#define DECODE_FLAGS_FORCE_DEBLOCK_FILTERING 512
#define DECODE_FLAGS_XUASTC_LDR_DISABLE_FAST_BC7_TRANSCODING 1024
