// MIT license see full LICENSE text at end of file
#pragma once
#ifndef TINY_DDS_TINYDDS_H
#define TINY_DDS_TINYDDS_H

#ifndef TINYDDS_HAVE_UINTXX_T
#include <stdint.h>  // for uint32_t and int64_t
#endif
#ifndef TINYDDS_HAVE_BOOL
#include <stdbool.h>  // for bool
#endif
#ifndef TINYDDS_HAVE_SIZE_T
#include <stddef.h>    // for size_t
#endif
#ifndef TINYDDS_HAVE_MEMCPY
#include <string.h>  // for memcpy
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TINYDDS_MAX_MIPMAPLEVELS 16

typedef struct TinyDDS_Context *TinyDDS_ContextHandle;

typedef void *(*TinyDDS_AllocFunc)(void *user, size_t size);
typedef void (*TinyDDS_FreeFunc)(void *user, void *memory);
typedef size_t (*TinyDDS_ReadFunc)(void *user, void *buffer, size_t byteCount);
typedef bool (*TinyDDS_SeekFunc)(void *user, int64_t offset);
typedef int64_t (*TinyDDS_TellFunc)(void *user);
typedef void (*TinyDDS_ErrorFunc)(void *user, char const *msg);

typedef struct TinyDDS_Callbacks {
	TinyDDS_ErrorFunc errorFn;
	TinyDDS_AllocFunc allocFn;
	TinyDDS_FreeFunc freeFn;
	TinyDDS_ReadFunc readFn;
	TinyDDS_SeekFunc seekFn;
	TinyDDS_TellFunc tellFn;
} TinyDDS_Callbacks;

TinyDDS_ContextHandle TinyDDS_CreateContext(TinyDDS_Callbacks const *callbacks, void *user);
void TinyDDS_DestroyContext(TinyDDS_ContextHandle handle);

// reset lets you reuse the context for another file (saves an alloc/free cycle)
void TinyDDS_Reset(TinyDDS_ContextHandle handle);

// call this to read the header file should already be at the start of the KTX data
bool TinyDDS_ReadHeader(TinyDDS_ContextHandle handle);

bool TinyDDS_Is1D(TinyDDS_ContextHandle handle);
bool TinyDDS_Is2D(TinyDDS_ContextHandle handle);
bool TinyDDS_Is3D(TinyDDS_ContextHandle handle);
bool TinyDDS_IsCubemap(TinyDDS_ContextHandle handle);
bool TinyDDS_IsArray(TinyDDS_ContextHandle handle);

bool TinyDDS_Dimensions(TinyDDS_ContextHandle handle,
												uint32_t *width,
												uint32_t *height,
												uint32_t *depth,
												uint32_t *slices);
uint32_t TinyDDS_Width(TinyDDS_ContextHandle handle);
uint32_t TinyDDS_Height(TinyDDS_ContextHandle handle);
uint32_t TinyDDS_Depth(TinyDDS_ContextHandle handle);
uint32_t TinyDDS_ArraySlices(TinyDDS_ContextHandle handle);

bool TinyDDS_NeedsGenerationOfMipmaps(TinyDDS_ContextHandle handle);
bool TinyDDS_NeedsEndianCorrecting(TinyDDS_ContextHandle handle);

uint32_t TinyDDS_NumberOfMipmaps(TinyDDS_ContextHandle handle);
uint32_t TinyDDS_ImageSize(TinyDDS_ContextHandle handle, uint32_t mipmaplevel);

// data return by ImageRawData is owned by the context. Don't free it!
void const *TinyDDS_ImageRawData(TinyDDS_ContextHandle handle, uint32_t mipmaplevel);

typedef void (*TinyDDS_WriteFunc)(void *user, void const *buffer, size_t byteCount);

typedef struct TinyDDS_WriteCallbacks {
	TinyDDS_ErrorFunc error;
	TinyDDS_AllocFunc alloc;
	TinyDDS_FreeFunc free;
	TinyDDS_WriteFunc write;
} TinyDDS_WriteCallbacks;

#ifndef TINYIMAGEFORMAT_DXGIFORMAT
#define TINYIMAGEFORMAT_DXGIFORMAT

// early DDS was a direct copy of the Draw Draw surface bits, later on (Dx10) it moved to
// DXGI_FORMAT we use a similar thing to DXGI_FORMAT second form but will synthesis
// the old style when required when saving and vice versa when loading.
typedef enum TinyImageFormat_DXGI_FORMAT {
	TIF_DXGI_FORMAT_UNKNOWN = 0,
	TIF_DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
	TIF_DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
	TIF_DXGI_FORMAT_R32G32B32A32_UINT = 3,
	TIF_DXGI_FORMAT_R32G32B32A32_SINT = 4,
	TIF_DXGI_FORMAT_R32G32B32_TYPELESS = 5,
	TIF_DXGI_FORMAT_R32G32B32_FLOAT = 6,
	TIF_DXGI_FORMAT_R32G32B32_UINT = 7,
	TIF_DXGI_FORMAT_R32G32B32_SINT = 8,
	TIF_DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
	TIF_DXGI_FORMAT_R16G16B16A16_FLOAT  = 10,
	TIF_DXGI_FORMAT_R16G16B16A16_UNORM = 11,
	TIF_DXGI_FORMAT_R16G16B16A16_UINT = 12,
	TIF_DXGI_FORMAT_R16G16B16A16_SNORM = 13,
	TIF_DXGI_FORMAT_R16G16B16A16_SINT = 14,
	TIF_DXGI_FORMAT_R32G32_TYPELESS = 15,
	TIF_DXGI_FORMAT_R32G32_FLOAT = 16,
	TIF_DXGI_FORMAT_R32G32_UINT = 17,
	TIF_DXGI_FORMAT_R32G32_SINT = 18,
	TIF_DXGI_FORMAT_R32G8X24_TYPELESS = 19,
	TIF_DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	TIF_DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	TIF_DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	TIF_DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
	TIF_DXGI_FORMAT_R10G10B10A2_UNORM = 24,
	TIF_DXGI_FORMAT_R10G10B10A2_UINT = 25,
	TIF_DXGI_FORMAT_R11G11B10_FLOAT = 26,
	TIF_DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
	TIF_DXGI_FORMAT_R8G8B8A8_UNORM = 28,
	TIF_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	TIF_DXGI_FORMAT_R8G8B8A8_UINT = 30,
	TIF_DXGI_FORMAT_R8G8B8A8_SNORM = 31,
	TIF_DXGI_FORMAT_R8G8B8A8_SINT = 32,
	TIF_DXGI_FORMAT_R16G16_TYPELESS = 33,
	TIF_DXGI_FORMAT_R16G16_FLOAT = 34,
	TIF_DXGI_FORMAT_R16G16_UNORM = 35,
	TIF_DXGI_FORMAT_R16G16_UINT = 36,
	TIF_DXGI_FORMAT_R16G16_SNORM = 37,
	TIF_DXGI_FORMAT_R16G16_SINT = 38,
	TIF_DXGI_FORMAT_R32_TYPELESS = 39,
	TIF_DXGI_FORMAT_D32_FLOAT = 40,
	TIF_DXGI_FORMAT_R32_FLOAT = 41,
	TIF_DXGI_FORMAT_R32_UINT = 42,
	TIF_DXGI_FORMAT_R32_SINT = 43,
	TIF_DXGI_FORMAT_R24G8_TYPELESS = 44,
	TIF_DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
	TIF_DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	TIF_DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
	TIF_DXGI_FORMAT_R8G8_TYPELESS = 48,
	TIF_DXGI_FORMAT_R8G8_UNORM = 49,
	TIF_DXGI_FORMAT_R8G8_UINT = 50,
	TIF_DXGI_FORMAT_R8G8_SNORM = 51,
	TIF_DXGI_FORMAT_R8G8_SINT = 52,
	TIF_DXGI_FORMAT_R16_TYPELESS = 53,
	TIF_DXGI_FORMAT_R16_FLOAT = 54,
	TIF_DXGI_FORMAT_D16_UNORM = 55,
	TIF_DXGI_FORMAT_R16_UNORM = 56,
	TIF_DXGI_FORMAT_R16_UINT = 57,
	TIF_DXGI_FORMAT_R16_SNORM = 58,
	TIF_DXGI_FORMAT_R16_SINT = 59,
	TIF_DXGI_FORMAT_R8_TYPELESS = 60,
	TIF_DXGI_FORMAT_R8_UNORM = 61,
	TIF_DXGI_FORMAT_R8_UINT = 62,
	TIF_DXGI_FORMAT_R8_SNORM = 63,
	TIF_DXGI_FORMAT_R8_SINT = 64,
	TIF_DXGI_FORMAT_A8_UNORM = 65,
	TIF_DXGI_FORMAT_R1_UNORM = 66,
	TIF_DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	TIF_DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
	TIF_DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
	TIF_DXGI_FORMAT_BC1_TYPELESS = 70,
	TIF_DXGI_FORMAT_BC1_UNORM = 71,
	TIF_DXGI_FORMAT_BC1_UNORM_SRGB = 72,
	TIF_DXGI_FORMAT_BC2_TYPELESS = 73,
	TIF_DXGI_FORMAT_BC2_UNORM = 74,
	TIF_DXGI_FORMAT_BC2_UNORM_SRGB = 75,
	TIF_DXGI_FORMAT_BC3_TYPELESS = 76,
	TIF_DXGI_FORMAT_BC3_UNORM = 77,
	TIF_DXGI_FORMAT_BC3_UNORM_SRGB = 78,
	TIF_DXGI_FORMAT_BC4_TYPELESS = 79,
	TIF_DXGI_FORMAT_BC4_UNORM = 80,
	TIF_DXGI_FORMAT_BC4_SNORM = 81,
	TIF_DXGI_FORMAT_BC5_TYPELESS = 82,
	TIF_DXGI_FORMAT_BC5_UNORM = 83,
	TIF_DXGI_FORMAT_BC5_SNORM = 84,
	TIF_DXGI_FORMAT_B5G6R5_UNORM = 85,
	TIF_DXGI_FORMAT_B5G5R5A1_UNORM = 86,
	TIF_DXGI_FORMAT_B8G8R8A8_UNORM = 87,
	TIF_DXGI_FORMAT_B8G8R8X8_UNORM = 88,
	TIF_DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	TIF_DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
	TIF_DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	TIF_DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
	TIF_DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	TIF_DXGI_FORMAT_BC6H_TYPELESS = 94,
	TIF_DXGI_FORMAT_BC6H_UF16 = 95,
	TIF_DXGI_FORMAT_BC6H_SF16 = 96,
	TIF_DXGI_FORMAT_BC7_TYPELESS = 97,
	TIF_DXGI_FORMAT_BC7_UNORM = 98,
	TIF_DXGI_FORMAT_BC7_UNORM_SRGB = 99,
	TIF_DXGI_FORMAT_AYUV = 100,
	TIF_DXGI_FORMAT_Y410 = 101,
	TIF_DXGI_FORMAT_Y416 = 102,
	TIF_DXGI_FORMAT_NV12 = 103,
	TIF_DXGI_FORMAT_P010 = 104,
	TIF_DXGI_FORMAT_P016 = 105,
	TIF_DXGI_FORMAT_420_OPAQUE = 106,
	TIF_DXGI_FORMAT_YUY2 = 107,
	TIF_DXGI_FORMAT_Y210 = 108,
	TIF_DXGI_FORMAT_Y216 = 109,
	TIF_DXGI_FORMAT_NV11 = 110,
	TIF_DXGI_FORMAT_AI44 = 111,
	TIF_DXGI_FORMAT_IA44 = 112,
	TIF_DXGI_FORMAT_P8 = 113,
	TIF_DXGI_FORMAT_A8P8 = 114,
	TIF_DXGI_FORMAT_B4G4R4A4_UNORM = 115,

	// xbox 360 formats
	TIF_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT = 116,
	TIF_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT = 117,
	TIF_DXGI_FORMAT_D16_UNORM_S8_UINT = 118,
	TIF_DXGI_FORMAT_R16_UNORM_X8_TYPELESS = 119,
	TIF_DXGI_FORMAT_X16_TYPELESS_G8_UINT = 120,

	TIF_DXGI_FORMAT_P208 = 130,
	TIF_DXGI_FORMAT_V208 = 131,
	TIF_DXGI_FORMAT_V408 = 132,

	// XBox One formats
	TIF_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM = 189,
	TIF_DXGI_FORMAT_R4G4_UNORM = 190,

} TinyImageFormat_DXGI_FORMAT;
#endif

typedef enum TinyDDS_Format {
	TDDS_UNDEFINED = TIF_DXGI_FORMAT_UNKNOWN,
	TDDS_B5G6R5_UNORM = TIF_DXGI_FORMAT_B5G6R5_UNORM,
	TDDS_B5G5R5A1_UNORM = TIF_DXGI_FORMAT_B5G5R5A1_UNORM,
	TDDS_R8_UNORM = TIF_DXGI_FORMAT_R8_UNORM,
	TDDS_R8_SNORM = TIF_DXGI_FORMAT_R8_SNORM,
	TDDS_A8_UNORM = TIF_DXGI_FORMAT_A8_UNORM,
	TDDS_R1_UNORM = TIF_DXGI_FORMAT_R1_UNORM,
	TDDS_R8_UINT = TIF_DXGI_FORMAT_R8_UINT,
	TDDS_R8_SINT = TIF_DXGI_FORMAT_R8_SINT,
	TDDS_R8G8_UNORM = TIF_DXGI_FORMAT_R8G8_UNORM,
	TDDS_R8G8_SNORM = TIF_DXGI_FORMAT_R8G8_SNORM,
	TDDS_R8G8_UINT = TIF_DXGI_FORMAT_R8G8_UINT,
	TDDS_R8G8_SINT = TIF_DXGI_FORMAT_R8G8_SINT,
	TDDS_R8G8B8A8_UNORM = TIF_DXGI_FORMAT_R8G8B8A8_UNORM,
	TDDS_R8G8B8A8_SNORM = TIF_DXGI_FORMAT_R8G8B8A8_SNORM,
	TDDS_R8G8B8A8_UINT = TIF_DXGI_FORMAT_R8G8B8A8_UINT,
	TDDS_R8G8B8A8_SINT = TIF_DXGI_FORMAT_R8G8B8A8_SINT,
	TDDS_R8G8B8A8_SRGB = TIF_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
	TDDS_B8G8R8A8_UNORM = TIF_DXGI_FORMAT_B8G8R8A8_UNORM,
	TDDS_B8G8R8A8_SRGB = TIF_DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,

	TDDS_R9G9B9E5_UFLOAT = TIF_DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
	TDDS_R10G10B10A2_UNORM = TIF_DXGI_FORMAT_R10G10B10A2_UNORM,
	TDDS_R10G10B10A2_UINT = TIF_DXGI_FORMAT_R10G10B10A2_UINT,
	TDDS_R11G11B10_UFLOAT = TIF_DXGI_FORMAT_R11G11B10_FLOAT,

	TDDS_R16_UNORM = TIF_DXGI_FORMAT_R16_UNORM,
	TDDS_R16_SNORM = TIF_DXGI_FORMAT_R16_SNORM,
	TDDS_R16_UINT = TIF_DXGI_FORMAT_R16_UINT,
	TDDS_R16_SINT = TIF_DXGI_FORMAT_R16_SINT,
	TDDS_R16_SFLOAT = TIF_DXGI_FORMAT_R16_FLOAT,

	TDDS_R16G16_UNORM = TIF_DXGI_FORMAT_R16G16_UNORM,
	TDDS_R16G16_SNORM = TIF_DXGI_FORMAT_R16G16_SNORM,
	TDDS_R16G16_UINT = TIF_DXGI_FORMAT_R16G16_UINT,
	TDDS_R16G16_SINT = TIF_DXGI_FORMAT_R16G16_SINT,
	TDDS_R16G16_SFLOAT = TIF_DXGI_FORMAT_R16G16_FLOAT,

	TDDS_R16G16B16A16_UNORM = TIF_DXGI_FORMAT_R16G16B16A16_UNORM,
	TDDS_R16G16B16A16_SNORM = TIF_DXGI_FORMAT_R16G16B16A16_SNORM,
	TDDS_R16G16B16A16_UINT = TIF_DXGI_FORMAT_R16G16B16A16_UINT,
	TDDS_R16G16B16A16_SINT = TIF_DXGI_FORMAT_R16G16B16A16_SINT,
	TDDS_R16G16B16A16_SFLOAT = TIF_DXGI_FORMAT_R16G16B16A16_FLOAT,

	TDDS_R32_UINT = TIF_DXGI_FORMAT_R32_UINT,
	TDDS_R32_SINT = TIF_DXGI_FORMAT_R32_SINT,
	TDDS_R32_SFLOAT = TIF_DXGI_FORMAT_R32_FLOAT,

	TDDS_R32G32_UINT = TIF_DXGI_FORMAT_R32G32_UINT,
	TDDS_R32G32_SINT = TIF_DXGI_FORMAT_R32G32_SINT,
	TDDS_R32G32_SFLOAT = TIF_DXGI_FORMAT_R32G32_FLOAT,

	TDDS_R32G32B32_UINT = TIF_DXGI_FORMAT_R32G32B32_UINT,
	TDDS_R32G32B32_SINT = TIF_DXGI_FORMAT_R32G32B32_SINT,
	TDDS_R32G32B32_SFLOAT = TIF_DXGI_FORMAT_R32G32B32_FLOAT,

	TDDS_R32G32B32A32_UINT = TIF_DXGI_FORMAT_R32G32B32A32_UINT,
	TDDS_R32G32B32A32_SINT = TIF_DXGI_FORMAT_R32G32B32A32_SINT,
	TDDS_R32G32B32A32_SFLOAT = TIF_DXGI_FORMAT_R32G32B32A32_FLOAT,

	TDDS_BC1_RGBA_UNORM_BLOCK = TIF_DXGI_FORMAT_BC1_UNORM,
	TDDS_BC1_RGBA_SRGB_BLOCK = TIF_DXGI_FORMAT_BC1_UNORM_SRGB,
	TDDS_BC2_UNORM_BLOCK = TIF_DXGI_FORMAT_BC2_UNORM,
	TDDS_BC2_SRGB_BLOCK = TIF_DXGI_FORMAT_BC2_UNORM_SRGB,
	TDDS_BC3_UNORM_BLOCK = TIF_DXGI_FORMAT_BC3_UNORM,
	TDDS_BC3_SRGB_BLOCK = TIF_DXGI_FORMAT_BC3_UNORM_SRGB,
	TDDS_BC4_UNORM_BLOCK = TIF_DXGI_FORMAT_BC4_UNORM,
	TDDS_BC4_SNORM_BLOCK = TIF_DXGI_FORMAT_BC4_SNORM,
	TDDS_BC5_UNORM_BLOCK = TIF_DXGI_FORMAT_BC5_UNORM,
	TDDS_BC5_SNORM_BLOCK = TIF_DXGI_FORMAT_BC5_SNORM,

	TDDS_BC6H_UFLOAT_BLOCK = TIF_DXGI_FORMAT_BC6H_UF16,
	TDDS_BC6H_SFLOAT_BLOCK = TIF_DXGI_FORMAT_BC6H_SF16,
	TDDS_BC7_UNORM_BLOCK = TIF_DXGI_FORMAT_BC7_UNORM,
	TDDS_BC7_SRGB_BLOCK = TIF_DXGI_FORMAT_BC7_UNORM_SRGB,

	TDDS_AYUV = TIF_DXGI_FORMAT_AYUV,
	TDDS_Y410 = TIF_DXGI_FORMAT_Y410,
	TDDS_Y416 = TIF_DXGI_FORMAT_Y416,
	TDDS_NV12 = TIF_DXGI_FORMAT_NV12,
	TDDS_P010 = TIF_DXGI_FORMAT_P010,
	TDDS_P016 = TIF_DXGI_FORMAT_P016,
	TDDS_420_OPAQUE = TIF_DXGI_FORMAT_420_OPAQUE,
	TDDS_YUY2 = TIF_DXGI_FORMAT_YUY2,
	TDDS_Y210 = TIF_DXGI_FORMAT_Y210,
	TDDS_Y216 = TIF_DXGI_FORMAT_Y216,
	TDDS_NV11 = TIF_DXGI_FORMAT_NV11,
	TDDS_AI44 = TIF_DXGI_FORMAT_AI44,
	TDDS_IA44 = TIF_DXGI_FORMAT_IA44,
	TDDS_P8 = TIF_DXGI_FORMAT_P8,
	TDDS_A8P8 = TIF_DXGI_FORMAT_A8P8,
	TDDS_B4G4R4A4_UNORM = TIF_DXGI_FORMAT_B4G4R4A4_UNORM,
	TDDS_R10G10B10_7E3_A2_FLOAT = TIF_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT,
	TDDS_R10G10B10_6E4_A2_FLOAT = TIF_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT,
	TDDS_D16_UNORM_S8_UINT = TIF_DXGI_FORMAT_D16_UNORM_S8_UINT,
	TDDS_R16_UNORM_X8_TYPELESS = TIF_DXGI_FORMAT_R16_UNORM_X8_TYPELESS,
	TDDS_X16_TYPELESS_G8_UINT = TIF_DXGI_FORMAT_X16_TYPELESS_G8_UINT,
	TDDS_P208 = TIF_DXGI_FORMAT_P208,
	TDDS_V208 = TIF_DXGI_FORMAT_V208,
	TDDS_V408 = TIF_DXGI_FORMAT_V408,
	TDDS_R10G10B10_SNORM_A2_UNORM = TIF_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM,
	TDDS_R4G4_UNORM = TIF_DXGI_FORMAT_R4G4_UNORM,

	TDDS_SYNTHESISED_DXGIFORMATS = 0xFFFF,
	TDDS_G4R4_UNORM = TDDS_SYNTHESISED_DXGIFORMATS,

	TDDS_A4B4G4R4_UNORM,
	TDDS_X4B4G4R4_UNORM,

	TDDS_A4R4G4B4_UNORM,
	TDDS_X4R4G4B4_UNORM,

	TDDS_B4G4R4X4_UNORM,

	TDDS_R4G4B4A4_UNORM,
	TDDS_R4G4B4X4_UNORM,

	TDDS_B5G5R5X1_UNORM,

	TDDS_R5G5B5A1_UNORM,
	TDDS_R5G5B5X1_UNORM,

	TDDS_A1R5G5B5_UNORM,
	TDDS_X1R5G5B5_UNORM,

	TDDS_A1B5G5R5_UNORM,
	TDDS_X1B5G5R5_UNORM,

	TDDS_R5G6B5_UNORM,

	TDDS_B2G3R3_UNORM,
	TDDS_B2G3R3A8_UNORM,

	TDDS_G8R8_UNORM,
	TDDS_G8R8_SNORM,

	TDDS_R8G8B8_UNORM,
	TDDS_B8G8R8_UNORM,

	TDDS_A8B8G8R8_SNORM,
	TDDS_B8G8R8A8_SNORM,

	TDDS_R8G8B8X8_UNORM,
	TDDS_B8G8R8X8_UNORM,
	TDDS_A8B8G8R8_UNORM,
	TDDS_X8B8G8R8_UNORM,
	TDDS_A8R8G8B8_UNORM,
	TDDS_X8R8G8B8_UNORM,

	TDDS_R10G10B10A2_SNORM,
	TDDS_B10G10R10A2_UNORM,
	TDDS_B10G10R10A2_SNORM,
	TDDS_A2B10G10R10_UNORM,
	TDDS_A2B10G10R10_SNORM,
	TDDS_A2R10G10B10_UNORM,
	TDDS_A2R10G10B10_SNORM,

	TDDS_G16R16_UNORM,
	TDDS_G16R16_SNORM,

} TinyDDS_Format;

// tiny_imageformat/format needs included before tinydds.h for this functionality
#ifdef TINYIMAGEFORMAT_BASE_H_

static TinyImageFormat TinyImageFormat_FromTinyDDSFormat(TinyDDS_Format fmt) {
	switch (fmt) {
	case TDDS_UNDEFINED: return TinyImageFormat_UNDEFINED;

	case TDDS_R32G32B32A32_SFLOAT: return TinyImageFormat_R32G32B32A32_SFLOAT;
	case TDDS_R32G32B32A32_UINT: return TinyImageFormat_R32G32B32A32_UINT;
	case TDDS_R32G32B32A32_SINT: return TinyImageFormat_R32G32B32A32_SINT;
	case TDDS_R32G32B32_SFLOAT: return TinyImageFormat_R32G32B32_SFLOAT;
	case TDDS_R32G32B32_UINT: return TinyImageFormat_R32G32B32_UINT;
	case TDDS_R32G32B32_SINT: return TinyImageFormat_R32G32B32_SINT;
	case TDDS_R16G16B16A16_SFLOAT: return TinyImageFormat_R16G16B16A16_SFLOAT;
	case TDDS_R16G16B16A16_UNORM: return TinyImageFormat_R16G16B16A16_UNORM;
	case TDDS_R16G16B16A16_UINT: return TinyImageFormat_R16G16B16A16_UINT;
	case TDDS_R16G16B16A16_SNORM: return TinyImageFormat_R16G16B16A16_SNORM;
	case TDDS_R16G16B16A16_SINT: return TinyImageFormat_R16G16B16A16_SINT;
	case TDDS_R32G32_SFLOAT: return TinyImageFormat_R32G32_SFLOAT;
	case TDDS_R32G32_UINT: return TinyImageFormat_R32G32_UINT;
	case TDDS_R32G32_SINT: return TinyImageFormat_R32G32_SINT;
	case TDDS_R8G8B8A8_UNORM: return TinyImageFormat_R8G8B8A8_UNORM;
	case TDDS_R8G8B8A8_SRGB: return TinyImageFormat_R8G8B8A8_SRGB;
	case TDDS_R8G8B8A8_UINT: return TinyImageFormat_R8G8B8A8_UINT;
	case TDDS_R8G8B8A8_SNORM: return TinyImageFormat_R8G8B8A8_SNORM;
	case TDDS_R8G8B8A8_SINT: return TinyImageFormat_R8G8B8A8_SINT;
	case TDDS_R16G16_SFLOAT: return TinyImageFormat_R16G16_SFLOAT;
	case TDDS_R16G16_UNORM: return TinyImageFormat_R16G16_UNORM;
	case TDDS_R16G16_UINT: return TinyImageFormat_R16G16_UINT;
	case TDDS_R16G16_SNORM: return TinyImageFormat_R16G16_SNORM;
	case TDDS_R16G16_SINT: return TinyImageFormat_R16G16_SINT;
	case TDDS_R32_SFLOAT: return TinyImageFormat_R32_SFLOAT;
	case TDDS_R32_UINT: return TinyImageFormat_R32_UINT;
	case TDDS_R32_SINT: return TinyImageFormat_R32_SINT;

	case TDDS_R8G8_UNORM: return TinyImageFormat_R8G8_UNORM;
	case TDDS_R8G8_UINT: return TinyImageFormat_R8G8_UINT;
	case TDDS_R8G8_SNORM: return TinyImageFormat_R8G8_SNORM;
	case TDDS_R8G8_SINT: return TinyImageFormat_R8G8_SINT;
	case TDDS_G8R8_UNORM: return TinyImageFormat_G8R8_UNORM;
	case TDDS_G8R8_SNORM: return TinyImageFormat_G8R8_SNORM;

	case TDDS_R16_SFLOAT: return TinyImageFormat_R16_SFLOAT;
	case TDDS_R16_UNORM: return TinyImageFormat_R16_UNORM;
	case TDDS_R16_UINT: return TinyImageFormat_R16_UINT;
	case TDDS_R16_SNORM: return TinyImageFormat_R16_SNORM;
	case TDDS_R16_SINT: return TinyImageFormat_R16_SINT;
	case TDDS_R8_UNORM: return TinyImageFormat_R8_UNORM;
	case TDDS_R8_UINT: return TinyImageFormat_R8_UINT;
	case TDDS_R8_SNORM: return TinyImageFormat_R8_SNORM;
	case TDDS_R8_SINT: return TinyImageFormat_R8_SINT;
	case TDDS_A8_UNORM: return TinyImageFormat_A8_UNORM;
	case TDDS_BC1_RGBA_UNORM_BLOCK: return TinyImageFormat_DXBC1_RGBA_UNORM;
	case TDDS_BC1_RGBA_SRGB_BLOCK:			return TinyImageFormat_DXBC1_RGBA_SRGB;
	case TDDS_BC2_UNORM_BLOCK: return TinyImageFormat_DXBC2_UNORM;
	case TDDS_BC2_SRGB_BLOCK: return TinyImageFormat_DXBC2_SRGB;
	case TDDS_BC3_UNORM_BLOCK: return TinyImageFormat_DXBC3_UNORM;
	case TDDS_BC3_SRGB_BLOCK: return TinyImageFormat_DXBC3_SRGB;
	case TDDS_BC4_UNORM_BLOCK: return TinyImageFormat_DXBC4_UNORM;
	case TDDS_BC4_SNORM_BLOCK: return TinyImageFormat_DXBC4_SNORM;
	case TDDS_BC5_UNORM_BLOCK: return TinyImageFormat_DXBC5_UNORM;
	case TDDS_BC5_SNORM_BLOCK: return TinyImageFormat_DXBC5_SNORM;
	case TDDS_BC6H_UFLOAT_BLOCK: return TinyImageFormat_DXBC6H_UFLOAT;
	case TDDS_BC6H_SFLOAT_BLOCK: return TinyImageFormat_DXBC6H_SFLOAT;
	case TDDS_BC7_UNORM_BLOCK: return TinyImageFormat_DXBC7_UNORM;
	case TDDS_BC7_SRGB_BLOCK: return TinyImageFormat_DXBC7_SRGB;
	case TDDS_B8G8R8A8_UNORM: return TinyImageFormat_B8G8R8A8_UNORM;
	case TDDS_B8G8R8A8_SRGB: return TinyImageFormat_B8G8R8A8_SRGB;

	case TDDS_B2G3R3A8_UNORM: return TinyImageFormat_B2G3R3A8_UNORM;
	case TDDS_B2G3R3_UNORM: return TinyImageFormat_B2G3R3_UNORM;
	case TDDS_R4G4_UNORM: return TinyImageFormat_R4G4_UNORM;

	case TDDS_R8G8B8_UNORM: return TinyImageFormat_R8G8B8_UNORM;
	case TDDS_B8G8R8_UNORM:	return TinyImageFormat_B8G8R8_UNORM;
	case TDDS_B8G8R8A8_SNORM:					return TinyImageFormat_B8G8R8A8_SNORM;

	case TDDS_R9G9B9E5_UFLOAT: return TinyImageFormat_E5B9G9R9_UFLOAT;
	case TDDS_R11G11B10_UFLOAT: return TinyImageFormat_B10G11R11_UFLOAT;
	case TDDS_G4R4_UNORM: return TinyImageFormat_G4R4_UNORM;

	case TDDS_R5G6B5_UNORM: return TinyImageFormat_R5G6B5_UNORM;
	case TDDS_B5G6R5_UNORM: return TinyImageFormat_B5G6R5_UNORM;

	case TDDS_B5G5R5A1_UNORM: return TinyImageFormat_B5G5R5A1_UNORM;
	case TDDS_B5G5R5X1_UNORM: return TinyImageFormat_B5G5R5X1_UNORM;

	case TDDS_R5G5B5A1_UNORM: return TinyImageFormat_R5G5B5A1_UNORM;
	case TDDS_R5G5B5X1_UNORM: return TinyImageFormat_R5G5B5X1_UNORM;

	case TDDS_A1R5G5B5_UNORM: return TinyImageFormat_A1R5G5B5_UNORM;
	case TDDS_X1R5G5B5_UNORM: return TinyImageFormat_X1R5G5B5_UNORM;

	case TDDS_X1B5G5R5_UNORM: return TinyImageFormat_X1B5G5R5_UNORM;
	case TDDS_A1B5G5R5_UNORM: return TinyImageFormat_A1B5G5R5_UNORM;

	case TDDS_X4B4G4R4_UNORM: return TinyImageFormat_X4B4G4R4_UNORM;
	case TDDS_X4R4G4B4_UNORM: return TinyImageFormat_X4R4G4B4_UNORM;
	case TDDS_A4R4G4B4_UNORM:	return TinyImageFormat_A4R4G4B4_UNORM;
	case TDDS_B4G4R4A4_UNORM: return TinyImageFormat_B4G4R4A4_UNORM;
	case TDDS_A4B4G4R4_UNORM: return TinyImageFormat_A4B4G4R4_UNORM;
	case TDDS_B4G4R4X4_UNORM: return TinyImageFormat_B4G4R4X4_UNORM;
	case TDDS_R4G4B4A4_UNORM: return TinyImageFormat_R4G4B4A4_UNORM;
	case TDDS_R4G4B4X4_UNORM:	return TinyImageFormat_R4G4B4X4_UNORM;

	case TDDS_R8G8B8X8_UNORM: return TinyImageFormat_R8G8B8X8_UNORM;

	// DDS A2R10B10G10 support is basically broken historically so expect channels to need swapping
	case TDDS_A2B10G10R10_UNORM:	return TinyImageFormat_A2B10G10R10_UNORM;
	case TDDS_A2B10G10R10_SNORM:	return TinyImageFormat_A2B10G10R10_SNORM;
	case TDDS_A2R10G10B10_UNORM:	return TinyImageFormat_A2R10G10B10_UNORM;
	case TDDS_A2R10G10B10_SNORM:	return TinyImageFormat_A2R10G10B10_SNORM;
	case TDDS_B10G10R10A2_UNORM: 	return TinyImageFormat_R10G10B10A2_UNORM;
	case TDDS_B10G10R10A2_SNORM: 	return TinyImageFormat_R10G10B10A2_SNORM;
	case TDDS_R10G10B10A2_UNORM: 	return TinyImageFormat_B10G10R10A2_UNORM;
	case TDDS_R10G10B10A2_SNORM: 	return TinyImageFormat_B10G10R10A2_SNORM;
	case TDDS_R10G10B10A2_UINT: 	return TinyImageFormat_B10G10R10A2_UINT;

	case TDDS_B8G8R8X8_UNORM: return TinyImageFormat_B8G8R8X8_UNORM;

	case TDDS_G16R16_UNORM: return TinyImageFormat_G16R16_UNORM;
	case TDDS_G16R16_SNORM: return TinyImageFormat_G16R16_SNORM;
	case TDDS_X8B8G8R8_UNORM: return TinyImageFormat_R8G8B8X8_UNORM;
	case TDDS_X8R8G8B8_UNORM: return TinyImageFormat_B8G8R8X8_UNORM;
	case TDDS_A8B8G8R8_UNORM: return TinyImageFormat_R8G8B8A8_UNORM;
	case TDDS_A8R8G8B8_UNORM: return TinyImageFormat_B8G8R8A8_UNORM;
	case TDDS_A8B8G8R8_SNORM: return TinyImageFormat_R8G8B8X8_UNORM;
	case TDDS_P8: return TinyImageFormat_CLUT_P8;
	case TDDS_A8P8: return TinyImageFormat_CLUT_P8A8;
	case TDDS_R1_UNORM: return TinyImageFormat_R1_UNORM;

	case TDDS_AYUV:break;
	case TDDS_Y410:break;
	case TDDS_Y416:break;
	case TDDS_NV12:break;
	case TDDS_P010:break;
	case TDDS_P016:break;
	case TDDS_420_OPAQUE:break;
	case TDDS_YUY2:break;
	case TDDS_Y210:break;
	case TDDS_Y216:break;
	case TDDS_NV11:break;
	case TDDS_AI44:break;
	case TDDS_IA44:break;
	case TDDS_R10G10B10_7E3_A2_FLOAT:break;
	case TDDS_R10G10B10_6E4_A2_FLOAT:break;
	case TDDS_D16_UNORM_S8_UINT:break;
	case TDDS_R16_UNORM_X8_TYPELESS:break;
	case TDDS_X16_TYPELESS_G8_UINT:break;
	case TDDS_P208:break;
	case TDDS_V208:break;
	case TDDS_V408:break;
	case TDDS_R10G10B10_SNORM_A2_UNORM:break;
	}

	return TinyImageFormat_UNDEFINED;
}

static TinyDDS_Format TinyImageFormat_ToTinyDDSFormat(TinyImageFormat fmt) {
	switch (fmt) {
	case TinyImageFormat_R4G4_UNORM: return TDDS_R4G4_UNORM;
	case TinyImageFormat_G4R4_UNORM: return TDDS_G4R4_UNORM;

	case TinyImageFormat_A4R4G4B4_UNORM: return TDDS_A4R4G4B4_UNORM;
	case TinyImageFormat_B4G4R4A4_UNORM: return TDDS_B4G4R4A4_UNORM;
	case TinyImageFormat_A4B4G4R4_UNORM: return TDDS_A4B4G4R4_UNORM;
	case TinyImageFormat_X4R4G4B4_UNORM: return TDDS_X4R4G4B4_UNORM;
	case TinyImageFormat_X4B4G4R4_UNORM: return TDDS_X4B4G4R4_UNORM;
	case TinyImageFormat_R4G4B4A4_UNORM: return TDDS_R4G4B4A4_UNORM;
	case TinyImageFormat_R4G4B4X4_UNORM: return TDDS_R4G4B4X4_UNORM;

	case TinyImageFormat_A1B5G5R5_UNORM: return TDDS_A1B5G5R5_UNORM;
	case TinyImageFormat_X1B5G5R5_UNORM: return TDDS_X1B5G5R5_UNORM;

	case TinyImageFormat_A1R5G5B5_UNORM: return TDDS_A1R5G5B5_UNORM;
	case TinyImageFormat_X1R5G5B5_UNORM: return TDDS_X1R5G5B5_UNORM;

	case TinyImageFormat_B5G5R5A1_UNORM: return TDDS_B5G5R5A1_UNORM;
	case TinyImageFormat_B5G5R5X1_UNORM: return TDDS_B5G5R5X1_UNORM;

	case TinyImageFormat_R5G5B5A1_UNORM: return TDDS_R5G5B5A1_UNORM;
	case TinyImageFormat_R5G5B5X1_UNORM: return TDDS_R5G5B5X1_UNORM;

	case TinyImageFormat_R5G6B5_UNORM: return TDDS_R5G6B5_UNORM;
	case TinyImageFormat_B5G6R5_UNORM: return TDDS_B5G6R5_UNORM;

	case TinyImageFormat_A2B10G10R10_UNORM:	return TDDS_A2B10G10R10_UNORM;
	case TinyImageFormat_A2B10G10R10_SNORM:	return TDDS_A2B10G10R10_SNORM;
	case TinyImageFormat_A2R10G10B10_UNORM:	return TDDS_A2R10G10B10_UNORM;
	case TinyImageFormat_A2R10G10B10_SNORM:	return TDDS_A2R10G10B10_SNORM;
	case TinyImageFormat_R10G10B10A2_UNORM: return TDDS_B10G10R10A2_UNORM;
	case TinyImageFormat_R10G10B10A2_SNORM: return TDDS_B10G10R10A2_SNORM;
	case TinyImageFormat_B10G10R10A2_UNORM: return TDDS_R10G10B10A2_UNORM;
	case TinyImageFormat_B10G10R10A2_SNORM: return TDDS_R10G10B10A2_SNORM;
	case TinyImageFormat_B10G10R10A2_UINT: 	return TDDS_R10G10B10A2_UINT;

	case TinyImageFormat_E5B9G9R9_UFLOAT: return TDDS_R9G9B9E5_UFLOAT;
	case TinyImageFormat_B10G11R11_UFLOAT: return TDDS_R11G11B10_UFLOAT;

	case TinyImageFormat_R8_UNORM: 				return TDDS_R8_UNORM;
	case TinyImageFormat_R8_SNORM: 				return TDDS_R8_SNORM;
	case TinyImageFormat_R8_UINT: 				return TDDS_R8_UINT;
	case TinyImageFormat_R8_SINT: 				return TDDS_R8_SINT;
	case TinyImageFormat_A8_UNORM: 				return TDDS_A8_UNORM;
	case TinyImageFormat_B2G3R3_UNORM: 		return TDDS_B2G3R3_UNORM;

	case TinyImageFormat_B2G3R3A8_UNORM: 	return TDDS_B2G3R3A8_UNORM;
	case TinyImageFormat_R8G8_UNORM: 			return TDDS_R8G8_UNORM;
	case TinyImageFormat_R8G8_SNORM: 			return TDDS_R8G8_SNORM;
	case TinyImageFormat_R8G8_UINT: 			return TDDS_R8G8_UINT;
	case TinyImageFormat_R8G8_SINT: 			return TDDS_R8G8_SINT;
	case TinyImageFormat_G8R8_UNORM:			return TDDS_G8R8_UNORM;
	case TinyImageFormat_G8R8_SNORM:			return TDDS_G8R8_SNORM;

	case TinyImageFormat_R8G8B8_UNORM: 		return TDDS_R8G8B8_UNORM;
	case TinyImageFormat_B8G8R8_UNORM:		return TDDS_B8G8R8_UNORM;

	case TinyImageFormat_R8G8B8A8_UNORM: 	return TDDS_R8G8B8A8_UNORM;
	case TinyImageFormat_R8G8B8A8_SNORM: 	return TDDS_R8G8B8A8_SNORM;
	case TinyImageFormat_R8G8B8A8_UINT: 	return TDDS_R8G8B8A8_UINT;
	case TinyImageFormat_R8G8B8A8_SINT: 	return TDDS_R8G8B8A8_SINT;
	case TinyImageFormat_R8G8B8A8_SRGB: 	return TDDS_R8G8B8A8_SRGB;
	case TinyImageFormat_B8G8R8A8_UNORM: 	return TDDS_B8G8R8A8_UNORM;
	case TinyImageFormat_B8G8R8A8_SRGB:		return TDDS_B8G8R8A8_SRGB;

	case TinyImageFormat_R16_UNORM:				return TDDS_R16_UNORM;
	case TinyImageFormat_R16_SNORM:				return TDDS_R16_SNORM;
	case TinyImageFormat_R16_UINT:				return TDDS_R16_UINT;
	case TinyImageFormat_R16_SINT:				return TDDS_R16_SINT;
	case TinyImageFormat_R16_SFLOAT:			return TDDS_R16_SFLOAT;

	case TinyImageFormat_R16G16_UNORM:		return TDDS_R16G16_UNORM;
	case TinyImageFormat_R16G16_SNORM:		return TDDS_R16G16_SNORM;
	case TinyImageFormat_R16G16_UINT:			return TDDS_R16G16_UINT;
	case TinyImageFormat_R16G16_SINT:			return TDDS_R16G16_SINT;
	case TinyImageFormat_R16G16_SFLOAT:		return TDDS_R16G16_SFLOAT;

	case TinyImageFormat_G16R16_UNORM:					return TDDS_G16R16_UNORM;
	case TinyImageFormat_G16R16_SNORM:					return TDDS_G16R16_SNORM;

	case TinyImageFormat_R16G16B16A16_UNORM:	return TDDS_R16G16B16A16_UNORM;
	case TinyImageFormat_R16G16B16A16_SNORM:	return TDDS_R16G16B16A16_SNORM;
	case TinyImageFormat_R16G16B16A16_UINT:		return TDDS_R16G16B16A16_UINT;
	case TinyImageFormat_R16G16B16A16_SINT:		return TDDS_R16G16B16A16_SINT;
	case TinyImageFormat_R16G16B16A16_SFLOAT:	return TDDS_R16G16B16A16_SFLOAT;

	case TinyImageFormat_R32_UINT:				return TDDS_R32_UINT;
	case TinyImageFormat_R32_SINT:				return TDDS_R32_SINT;
	case TinyImageFormat_R32_SFLOAT:			return TDDS_R32_SFLOAT;

	case TinyImageFormat_R32G32_UINT:			return TDDS_R32G32_UINT;
	case TinyImageFormat_R32G32_SINT:			return TDDS_R32G32_SINT;
	case TinyImageFormat_R32G32_SFLOAT:		return TDDS_R32G32_SFLOAT;

	case TinyImageFormat_R32G32B32_UINT:	return TDDS_R32G32B32_UINT;
	case TinyImageFormat_R32G32B32_SINT:	return TDDS_R32G32B32_SINT;
	case TinyImageFormat_R32G32B32_SFLOAT:return TDDS_R32G32B32_SFLOAT;

	case TinyImageFormat_R32G32B32A32_UINT:		return TDDS_R32G32B32A32_UINT;
	case TinyImageFormat_R32G32B32A32_SINT:		return TDDS_R32G32B32A32_SINT;
	case TinyImageFormat_R32G32B32A32_SFLOAT:	return TDDS_R32G32B32A32_SFLOAT;

	case TinyImageFormat_D16_UNORM:				return TDDS_R16_UNORM;
	case TinyImageFormat_D32_SFLOAT:			return TDDS_R32_SFLOAT;
	case TinyImageFormat_S8_UINT:					return TDDS_R8_UINT;
	case TinyImageFormat_DXBC1_RGB_UNORM: 	return TDDS_BC1_RGBA_UNORM_BLOCK;
	case TinyImageFormat_DXBC1_RGB_SRGB:		return TDDS_BC1_RGBA_SRGB_BLOCK;
	case TinyImageFormat_DXBC1_RGBA_UNORM:	return TDDS_BC1_RGBA_UNORM_BLOCK;
	case TinyImageFormat_DXBC1_RGBA_SRGB:		return TDDS_BC1_RGBA_SRGB_BLOCK;
	case TinyImageFormat_DXBC2_UNORM:				return TDDS_BC2_UNORM_BLOCK;
	case TinyImageFormat_DXBC2_SRGB:				return TDDS_BC2_SRGB_BLOCK;
	case TinyImageFormat_DXBC3_UNORM:				return TDDS_BC3_UNORM_BLOCK;
	case TinyImageFormat_DXBC3_SRGB:				return TDDS_BC3_SRGB_BLOCK;
	case TinyImageFormat_DXBC4_UNORM:				return TDDS_BC4_UNORM_BLOCK;
	case TinyImageFormat_DXBC4_SNORM:				return TDDS_BC4_SNORM_BLOCK;
	case TinyImageFormat_DXBC5_UNORM:				return TDDS_BC5_UNORM_BLOCK;
	case TinyImageFormat_DXBC5_SNORM:				return TDDS_BC5_SNORM_BLOCK;
	case TinyImageFormat_DXBC6H_UFLOAT:			return TDDS_BC6H_UFLOAT_BLOCK;
	case TinyImageFormat_DXBC6H_SFLOAT:			return TDDS_BC6H_SFLOAT_BLOCK;
	case TinyImageFormat_DXBC7_UNORM:				return TDDS_BC7_UNORM_BLOCK;
	case TinyImageFormat_DXBC7_SRGB:				return TDDS_BC7_SRGB_BLOCK;

	case TinyImageFormat_CLUT_P8: return TDDS_P8;
	case TinyImageFormat_CLUT_P8A8: return TDDS_A8P8;
	case TinyImageFormat_R1_UNORM: return TDDS_R1_UNORM;

		// unsupported
	// TODO Some of these can be via Dx10/4CC codes I think
	default: return TDDS_UNDEFINED;
	}

	return TDDS_UNDEFINED;
}
#endif

TinyDDS_Format TinyDDS_GetFormat(TinyDDS_ContextHandle handle);

bool TinyDDS_WriteImage(TinyDDS_WriteCallbacks const *callbacks,
												void *user,
												uint32_t width,
												uint32_t height,
												uint32_t depth,
												uint32_t slices,
												uint32_t mipmaplevels,
												TinyDDS_Format format,
												bool cubemap,
												bool preferDx10Format,
												uint32_t const *mipmapsizes,
												void const **mipmaps);

#ifdef TINYDDS_IMPLEMENTATION

#define TINYDDS_DDSD_CAPS 0x00000001
#define TINYDDS_DDSD_HEIGHT 0x00000002
#define TINYDDS_DDSD_WIDTH 0x00000004
#define TINYDDS_DDSD_PITCH 0x00000008
#define TINYDDS_DDSD_PIXELFORMAT 0x00001000
#define TINYDDS_DDSD_MIPMAPCOUNT 0x00020000
#define TINYDDS_DDSD_LINEARSIZE 0x00080000
#define TINYDDS_DDSD_DEPTH 0x00800000
#define TINYDDS_DDSCAPS_COMPLEX 0x00000008
#define TINYDDS_DDSCAPS_TEXTURE 0x00001000
#define TINYDDS_DDSCAPS_MIPMAP 0x00400000
#define TINYDDS_DDSCAPS2_CUBEMAP 0x00000200
#define TINYDDS_DDSCAPS2_VOLUME 0x00200000
#define TINYDDS_DDSCAPS2_CUBEMAP_ALL 0x0000FC000
#define TINYDDS_D3D10_RESOURCE_MISC_TEXTURECUBE 0x4
#define TINYDDS_D3D10_RESOURCE_DIMENSION_BUFFER 1
#define TINYDDS_D3D10_RESOURCE_DIMENSION_TEXTURE1D 2
#define TINYDDS_D3D10_RESOURCE_DIMENSION_TEXTURE2D 3
#define TINYDDS_D3D10_RESOURCE_DIMENSION_TEXTURE3D 4
#define TINYDDS_DDPF_ALPHAPIXELS                        0x00000001l
#define TINYDDS_DDPF_ALPHA                              0x00000002l
#define TINYDDS_DDPF_FOURCC                             0x00000004l
#define TINYDDS_DDPF_PALETTEINDEXED4                    0x00000008l
#define TINYDDS_DDPF_PALETTEINDEXEDTO8                  0x00000010l
#define TINYDDS_DDPF_PALETTEINDEXED8                    0x00000020l
#define TINYDDS_DDPF_RGB                                0x00000040l
#define TINYDDS_DDPF_LUMINANCE                          0x00020000l
#define TINYDDS_DDPF_BUMPLUMINANCE                      0x00040000l
#define TINYDDS_DDPF_BUMPDUDV                           0x00080000l

// some of these get stuck in unofficial DDS v9 FourCC code
typedef enum TINYDDS_D3DFORMAT {
	TINYDDS_D3DFMT_UNKNOWN              =  0,
	TINYDDS_D3DFMT_R8G8B8               = 20,
	TINYDDS_D3DFMT_A8R8G8B8             = 21,
	TINYDDS_D3DFMT_X8R8G8B8             = 22,
	TINYDDS_D3DFMT_R5G6B5               = 23,
	TINYDDS_D3DFMT_X1R5G5B5             = 24,
	TINYDDS_D3DFMT_A1R5G5B5             = 25,
	TINYDDS_D3DFMT_A4R4G4B4             = 26,
	TINYDDS_D3DFMT_R3G3B2               = 27,
	TINYDDS_D3DFMT_A8                   = 28,
	TINYDDS_D3DFMT_A8R3G3B2             = 29,
	TINYDDS_D3DFMT_X4R4G4B4             = 30,
	TINYDDS_D3DFMT_A2B10G10R10          = 31,
	TINYDDS_D3DFMT_A8B8G8R8             = 32,
	TINYDDS_D3DFMT_X8B8G8R8             = 33,
	TINYDDS_D3DFMT_G16R16               = 34,
	TINYDDS_D3DFMT_A2R10G10B10          = 35,
	TINYDDS_D3DFMT_A16B16G16R16         = 36,
	TINYDDS_D3DFMT_A8P8                 = 40,
	TINYDDS_D3DFMT_P8                   = 41,
	TINYDDS_D3DFMT_L8                   = 50,
	TINYDDS_D3DFMT_A8L8                 = 51,
	TINYDDS_D3DFMT_A4L4                 = 52,
	TINYDDS_D3DFMT_V8U8                 = 60,
	TINYDDS_D3DFMT_L6V5U5               = 61,
	TINYDDS_D3DFMT_X8L8V8U8             = 62,
	TINYDDS_D3DFMT_Q8W8V8U8             = 63,
	TINYDDS_D3DFMT_V16U16               = 64,
	TINYDDS_D3DFMT_A2W10V10U10          = 67,
	TINYDDS_D3DFMT_L16                  = 81,
	TINYDDS_D3DFMT_Q16W16V16U16         = 110,
	TINYDDS_D3DFMT_R16F                 = 111,
	TINYDDS_D3DFMT_G16R16F              = 112,
	TINYDDS_D3DFMT_A16B16G16R16F        = 113,
	TINYDDS_D3DFMT_R32F                 = 114,
	TINYDDS_D3DFMT_G32R32F              = 115,
	TINYDDS_D3DFMT_A32B32G32R32F        = 116,
	TINYDDS_D3DFMT_CxV8U8               = 117,
	TINYDDS_D3DFMT_A1                   = 118,
	TINYDDS_D3DFMT_A2B10G10R10_XR_BIAS  = 119,
} TINYDDS_D3DFORMAT;

typedef struct TinyDDS_Header {
	uint32_t magic;
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitchOrLinearSize;
	uint32_t depth;
	uint32_t mipMapCount;
	uint32_t reserved0[11];

	uint32_t formatSize;
	uint32_t formatFlags;
	uint32_t formatFourCC;
	uint32_t formatRGBBitCount;
	uint32_t formatRBitMask;
	uint32_t formatGBitMask;
	uint32_t formatBBitMask;
	uint32_t formatABitMask;

	uint32_t caps1;
	uint32_t caps2;
	uint32_t caps3; // not used?
	uint32_t caps4; // not used?

	uint32_t reserved1;
} TinyDDS_Header;

typedef struct TinyDDS_HeaderDX10 {
	uint32_t DXGIFormat;
	uint32_t resourceDimension;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t reserved;
} TinyDDS_HeaderDX10;

typedef struct TinyDDS_Context {
	TinyDDS_Callbacks callbacks;
	void *user;
	uint64_t headerPos;
	uint64_t firstImagePos;

	TinyDDS_Header header;
	TinyDDS_HeaderDX10 headerDx10;
	TinyDDS_Format format;

	bool headerValid;
	uint8_t const *mipmaps[TINYDDS_MAX_MIPMAPLEVELS];
	uint32_t const *clut;

} TinyDDS_Context;

#define TINYDDS_MAKE_RIFFCODE(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

//static uint32_t TinyDDS_fileIdentifier = TINYDDS_MAKE_RIFFCODE('D', 'D', 'S', ' ');

static void TinyDDS_NullErrorFunc(void *user, char const *msg) { BASISU_NOTE_UNUSED(user); BASISU_NOTE_UNUSED(msg); }

TinyDDS_ContextHandle TinyDDS_CreateContext(TinyDDS_Callbacks const *callbacks, void *user) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) callbacks->allocFn(user, sizeof(TinyDDS_Context));
	if (ctx == NULL)
		return NULL;

	memset(ctx, 0, sizeof(TinyDDS_Context));
	memcpy(&ctx->callbacks, callbacks, sizeof(TinyDDS_Callbacks));
	ctx->user = user;
	if (ctx->callbacks.errorFn == NULL) {
		ctx->callbacks.errorFn = &TinyDDS_NullErrorFunc;
	}

	if (ctx->callbacks.readFn == NULL) {
		ctx->callbacks.errorFn(user, "TinyDDS must have read callback");
		return NULL;
	}
	if (ctx->callbacks.allocFn == NULL) {
		ctx->callbacks.errorFn(user, "TinyDDS must have alloc callback");
		return NULL;
	}
	if (ctx->callbacks.freeFn == NULL) {
		ctx->callbacks.errorFn(user, "TinyDDS must have free callback");
		return NULL;
	}
	if (ctx->callbacks.seekFn == NULL) {
		ctx->callbacks.errorFn(user, "TinyDDS must have seek callback");
		return NULL;
	}
	if (ctx->callbacks.tellFn == NULL) {
		ctx->callbacks.errorFn(user, "TinyDDS must have tell callback");
		return NULL;
	}

	TinyDDS_Reset(ctx);

	return ctx;
}

void TinyDDS_DestroyContext(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return;
	TinyDDS_Reset(handle);

	ctx->callbacks.freeFn(ctx->user, ctx);
}

void TinyDDS_Reset(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return;

	// backup user provided callbacks and data
	TinyDDS_Callbacks callbacks;
	memcpy(&callbacks, &ctx->callbacks, sizeof(TinyDDS_Callbacks));
	void *user = ctx->user;

	for (int i = 0; i < TINYDDS_MAX_MIPMAPLEVELS; ++i) {
		if (ctx->mipmaps[i] != NULL) {
			callbacks.freeFn(user, (void *) ctx->mipmaps[i]);
		}
	}

	if(ctx->clut) {
		callbacks.freeFn(user, (void *) ctx->clut);
		ctx->clut = NULL;
	}

	// reset to default state
	memset(ctx, 0, sizeof(TinyDDS_Context));
	memcpy(&ctx->callbacks, &callbacks, sizeof(TinyDDS_Callbacks));
	ctx->user = user;

}

static bool TinyDDS_IsCLUT(TinyDDS_Format fmt) {
	switch (fmt) {
	case TDDS_P8:
	case TDDS_A8P8:
		return true;
	default: return false;
	}
}

static bool TinyDDS_IsCompressed(TinyDDS_Format fmt) {
	switch (fmt) {
	case TDDS_BC1_RGBA_UNORM_BLOCK:
	case TDDS_BC1_RGBA_SRGB_BLOCK:
	case TDDS_BC2_UNORM_BLOCK:
	case TDDS_BC2_SRGB_BLOCK:
	case TDDS_BC3_UNORM_BLOCK:
	case TDDS_BC3_SRGB_BLOCK:
	case TDDS_BC4_UNORM_BLOCK:
	case TDDS_BC4_SNORM_BLOCK:
	case TDDS_BC5_UNORM_BLOCK:
	case TDDS_BC5_SNORM_BLOCK:
	case TDDS_BC6H_UFLOAT_BLOCK:
	case TDDS_BC6H_SFLOAT_BLOCK:
	case TDDS_BC7_UNORM_BLOCK:
	case TDDS_BC7_SRGB_BLOCK: return true;
	default: return false;
	}
}

// the size is per pixel (except R1) for uncompressed and per block of 16 pixels for compressed
static uint32_t TinyDDS_FormatSize(TinyDDS_Format fmt) {
	switch(fmt) {
	// 8 pixels at 1 bits each
	case TDDS_R1_UNORM:
		return 1;
		// 2 * 4 bits
	case TDDS_R4G4_UNORM:
	case TDDS_G4R4_UNORM:
		// 1 * 8 bits
	case TDDS_P8:;
	case TDDS_R8_UNORM:
	case TDDS_R8_SNORM:
	case TDDS_R8_UINT:
	case TDDS_R8_SINT:
	case TDDS_A8_UNORM:
	// 2 + 2 * 3 bits
	case TDDS_B2G3R3_UNORM:
		return 1;

		// 2 + 2 * 3 +8 bits
	case TDDS_B2G3R3A8_UNORM:
	// 4 * 4 bits
	case TDDS_B4G4R4A4_UNORM:
	case TDDS_A4B4G4R4_UNORM:
	case TDDS_X4B4G4R4_UNORM:
	case TDDS_A4R4G4B4_UNORM:
	case TDDS_X4R4G4B4_UNORM:
	case TDDS_B4G4R4X4_UNORM:
	case TDDS_R4G4B4A4_UNORM:
	case TDDS_R4G4B4X4_UNORM:

		// 3 * 5 bits + 1 bit
	case TDDS_B5G5R5A1_UNORM:
	case TDDS_B5G5R5X1_UNORM:
	case TDDS_R5G5B5A1_UNORM:
	case TDDS_R5G5B5X1_UNORM:
	case TDDS_A1R5G5B5_UNORM:
	case TDDS_X1R5G5B5_UNORM:
	case TDDS_A1B5G5R5_UNORM:
	case TDDS_X1B5G5R5_UNORM:

		// 1 * 6 bit + 2 * 5 bits
	case TDDS_R5G6B5_UNORM:
	case TDDS_B5G6R5_UNORM:
		// 2 x 8 bits
	case TDDS_A8P8:
	case TDDS_R8G8_UNORM:
	case TDDS_R8G8_SNORM:
	case TDDS_G8R8_UNORM:
	case TDDS_G8R8_SNORM:
	case TDDS_R8G8_UINT:
	case TDDS_R8G8_SINT:
		// 1 * 16 bits
	case TDDS_R16_UNORM:
	case TDDS_R16_SNORM:
	case TDDS_R16_UINT:
	case TDDS_R16_SINT:
	case TDDS_R16_SFLOAT:
		return 2;

		// 3 * 8 bits
	case TDDS_R8G8B8_UNORM:
	case TDDS_B8G8R8_UNORM:
		return 3;
	// 4 * 8 bits
	case TDDS_A8B8G8R8_SNORM:
	case TDDS_R8G8B8A8_SNORM:
	case TDDS_R8G8B8A8_UINT:
	case TDDS_R8G8B8A8_SINT:
	case TDDS_R8G8B8A8_SRGB:
	case TDDS_B8G8R8A8_SRGB:
	case TDDS_B8G8R8A8_SNORM:

	case TDDS_R8G8B8A8_UNORM:
	case TDDS_R8G8B8X8_UNORM:
	case TDDS_B8G8R8A8_UNORM:
	case TDDS_B8G8R8X8_UNORM:
	case TDDS_A8B8G8R8_UNORM:
	case TDDS_X8B8G8R8_UNORM:
	case TDDS_A8R8G8B8_UNORM:
	case TDDS_X8R8G8B8_UNORM:

		// 3 * 9 bits + 5 bits
	case TDDS_R9G9B9E5_UFLOAT:
		// 3 * 10 bits + 2 bits
	case TDDS_R10G10B10_7E3_A2_FLOAT:
	case TDDS_R10G10B10_6E4_A2_FLOAT:
	case TDDS_R10G10B10_SNORM_A2_UNORM:

	case TDDS_B10G10R10A2_UNORM:
	case TDDS_B10G10R10A2_SNORM:
	case TDDS_A2B10G10R10_UNORM:
	case TDDS_A2B10G10R10_SNORM:
	case TDDS_A2R10G10B10_UNORM:
	case TDDS_A2R10G10B10_SNORM:
	case TDDS_R10G10B10A2_UNORM:
	case TDDS_R10G10B10A2_SNORM:
	case TDDS_R10G10B10A2_UINT:

		// 2 * 11 bits + 10 bits
	case TDDS_R11G11B10_UFLOAT:
		// 2 * 16 bits
	case TDDS_R16G16_UNORM:
	case TDDS_R16G16_SNORM:
	case TDDS_R16G16_UINT:
	case TDDS_R16G16_SINT:
	case TDDS_R16G16_SFLOAT:
	case TDDS_G16R16_UNORM:
	case TDDS_G16R16_SNORM:
		// 1 * 32 bits
	case TDDS_R32_UINT:
	case TDDS_R32_SINT:
	case TDDS_R32_SFLOAT:
		return 4;
		// 4 * 16 bits
	case TDDS_R16G16B16A16_UNORM:
	case TDDS_R16G16B16A16_SNORM:
	case TDDS_R16G16B16A16_UINT:
	case TDDS_R16G16B16A16_SINT:
	case TDDS_R16G16B16A16_SFLOAT:
		// 2 * 32 bits
	case TDDS_R32G32_UINT:
	case TDDS_R32G32_SINT:
	case TDDS_R32G32_SFLOAT:
		return 8;
		// 3 * 32 bits
	case TDDS_R32G32B32_UINT:
	case TDDS_R32G32B32_SINT:
	case TDDS_R32G32B32_SFLOAT:
		return 12;
		// 4 * 32 bits
	case TDDS_R32G32B32A32_UINT:
	case TDDS_R32G32B32A32_SINT:
	case TDDS_R32G32B32A32_SFLOAT:
		return 16;
		// block formats
	case TDDS_BC1_RGBA_UNORM_BLOCK:
	case TDDS_BC1_RGBA_SRGB_BLOCK:
	case TDDS_BC4_UNORM_BLOCK:
	case TDDS_BC4_SNORM_BLOCK:
		return 8;

 	case TDDS_BC2_UNORM_BLOCK:
	case TDDS_BC2_SRGB_BLOCK:
	case TDDS_BC3_UNORM_BLOCK:
	case TDDS_BC3_SRGB_BLOCK:
	case TDDS_BC5_UNORM_BLOCK:
	case TDDS_BC5_SNORM_BLOCK:
	case TDDS_BC6H_UFLOAT_BLOCK:
	case TDDS_BC6H_SFLOAT_BLOCK:
	case TDDS_BC7_UNORM_BLOCK:
	case TDDS_BC7_SRGB_BLOCK:
		return 16;

	case TDDS_UNDEFINED: return 0;
		//	default: return 0;
	case TDDS_AYUV:break;
	case TDDS_Y410:break;
	case TDDS_Y416:break;
	case TDDS_NV12:break;
	case TDDS_P010:break;
	case TDDS_P016:break;
	case TDDS_420_OPAQUE:break;
	case TDDS_YUY2:break;
	case TDDS_Y210:break;
	case TDDS_Y216:break;
	case TDDS_NV11:break;
	case TDDS_AI44:break;
	case TDDS_IA44:break;
	case TDDS_D16_UNORM_S8_UINT:break;
	case TDDS_R16_UNORM_X8_TYPELESS:break;
	case TDDS_X16_TYPELESS_G8_UINT:break;
	case TDDS_P208:break;
	case TDDS_V208:break;
	case TDDS_V408:break;
	}
	return 0;
}

#define TINYDDS_CHK_DDSFORMAT(bits, rm, gm, bm, am, fmt) \
			if ((ctx->header.formatRGBBitCount == bits) && \
					(ctx->header.formatRBitMask == rm) && \
					(ctx->header.formatGBitMask == gm) && \
					(ctx->header.formatBBitMask == bm) && \
					(ctx->header.formatABitMask == am)) { return fmt; }

static TinyDDS_Format TinyDDS_DecodeFormat(TinyDDS_Context *ctx) {
	if (ctx->header.formatFlags & TINYDDS_DDPF_FOURCC) {
		if (ctx->headerDx10.DXGIFormat != TIF_DXGI_FORMAT_UNKNOWN) {
			return (TinyDDS_Format) ctx->headerDx10.DXGIFormat;
		}

		// check fourCC and some special numbers..
		// unofficially during the dx9 timeline, D3D_FORMAT were stuck directly into
		// formatFourCC field we handle FourCC and these < 119 codes here
		// its unclear if this was only for formats that couldn't be exposed via
		// Direct Draw Surfaces (like floats etc.) so I decode most of them anyway
		switch (ctx->header.formatFourCC) {
		case TINYDDS_D3DFMT_R8G8B8: return TDDS_R8G8B8_UNORM;
		case TINYDDS_D3DFMT_A8R8G8B8: return TDDS_A8R8G8B8_UNORM;
		case TINYDDS_D3DFMT_X8R8G8B8: return TDDS_X8R8G8B8_UNORM;
		case TINYDDS_D3DFMT_R5G6B5: return TDDS_R5G6B5_UNORM;
		case TINYDDS_D3DFMT_X1R5G5B5: return TDDS_X1R5G5B5_UNORM;
		case TINYDDS_D3DFMT_A1R5G5B5: return TDDS_A1R5G5B5_UNORM;
		case TINYDDS_D3DFMT_A4R4G4B4: return TDDS_A4R4G4B4_UNORM;
		case TINYDDS_D3DFMT_R3G3B2: return TDDS_B2G3R3_UNORM;
		case TINYDDS_D3DFMT_A8: return TDDS_A8_UNORM;
		case TINYDDS_D3DFMT_A8R3G3B2: return TDDS_B2G3R3A8_UNORM;
		case TINYDDS_D3DFMT_X4R4G4B4: return TDDS_A4R4G4B4_UNORM;
		case TINYDDS_D3DFMT_A2B10G10R10: return TDDS_A2B10G10R10_UNORM;
		case TINYDDS_D3DFMT_A8B8G8R8: return TDDS_A8B8G8R8_UNORM;
		case TINYDDS_D3DFMT_X8B8G8R8: return TDDS_A8B8G8R8_UNORM;
		case TINYDDS_D3DFMT_A2R10G10B10: return TDDS_A2R10G10B10_UNORM;
		case TINYDDS_D3DFMT_G16R16: return TDDS_R16G16_UNORM;
		case TINYDDS_D3DFMT_A16B16G16R16: return TDDS_R16G16B16A16_UNORM;
		case TINYDDS_D3DFMT_R16F: return TDDS_R16_SFLOAT;
		case TINYDDS_D3DFMT_G16R16F: return TDDS_R16G16_SFLOAT;
		case TINYDDS_D3DFMT_A16B16G16R16F: return TDDS_R16G16B16A16_SFLOAT;
		case TINYDDS_D3DFMT_A8P8: return TDDS_A8P8;
		case TINYDDS_D3DFMT_P8: return TDDS_P8;
		case TINYDDS_D3DFMT_L8: return TDDS_R8_UNORM;
		case TINYDDS_D3DFMT_A8L8: return TDDS_R8G8_UNORM;
		case TINYDDS_D3DFMT_A4L4: return TDDS_R4G4_UNORM;
		case TINYDDS_D3DFMT_V8U8: return TDDS_G8R8_SNORM;
		case TINYDDS_D3DFMT_L6V5U5: return TDDS_UNDEFINED; // TODO TDDS_R5G6B5_SNORM_PACK16;
		case TINYDDS_D3DFMT_X8L8V8U8: return TDDS_R8G8B8A8_SNORM;
		case TINYDDS_D3DFMT_Q8W8V8U8: return TDDS_R8G8B8A8_SNORM;
		case TINYDDS_D3DFMT_V16U16: return TDDS_R16G16_SNORM;
		case TINYDDS_D3DFMT_A2W10V10U10: return TDDS_A2B10G10R10_SNORM;
		case TINYDDS_D3DFMT_L16: return TDDS_R16_UNORM;
		case TINYDDS_D3DFMT_Q16W16V16U16: return TDDS_R16G16B16A16_SNORM;
		case TINYDDS_D3DFMT_R32F: return TDDS_R32_SFLOAT;
		case TINYDDS_D3DFMT_G32R32F: return TDDS_R32G32_SFLOAT;
		case TINYDDS_D3DFMT_A32B32G32R32F: return TDDS_R32G32B32A32_SFLOAT;
		case TINYDDS_D3DFMT_CxV8U8: return TDDS_UNDEFINED;
		case TINYDDS_D3DFMT_A1: return TDDS_R1_UNORM;
		case TINYDDS_D3DFMT_A2B10G10R10_XR_BIAS: return TDDS_UNDEFINED;

			// real 4CC no exotics yet just the block compression ones
		case TINYDDS_MAKE_RIFFCODE('D', 'X', 'T', '1'): return TDDS_BC1_RGBA_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('D', 'X', 'T', '2'): return TDDS_BC2_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('D', 'X', 'T', '3'): return TDDS_BC2_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('D', 'X', 'T', '4'): return TDDS_BC3_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('D', 'X', 'T', '5'): return TDDS_BC3_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('A', 'T', 'I', '1'): return TDDS_BC4_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('A', 'T', 'I', '2'): return TDDS_BC5_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('B', 'C', '4', 'U'): return TDDS_BC4_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('B', 'C', '4', 'S'): return TDDS_BC4_SNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('B', 'C', '5', 'U'): return TDDS_BC5_UNORM_BLOCK;
		case TINYDDS_MAKE_RIFFCODE('B', 'C', '5', 'S'): return TDDS_BC5_SNORM_BLOCK;
		}
	}

	// okay back to direct draw surface bit fields to try and work format out.
	// TODO this could be better i'm sure

	if ((ctx->header.formatFlags & TINYDDS_DDPF_PALETTEINDEXED4)) {
		return TDDS_UNDEFINED; // TODO 4 bit CLUTs
	}

	if ((ctx->header.formatFlags & TINYDDS_DDPF_PALETTEINDEXED8)) {
		if(ctx->header.formatRGBBitCount != 8) return TDDS_UNDEFINED;
		if(ctx->header.formatFlags & TINYDDS_DDPF_ALPHA) {
			return TDDS_A8P8;
		} else {
			return TDDS_P8;
		}
	}
	// what is this? TINYDDS_DDPF_PALETTEINDEXEDTO8

	// most have RGB data and/or alpha
	if ((ctx->header.formatFlags & TINYDDS_DDPF_RGB) ||
			(ctx->header.formatFlags & TINYDDS_DDPF_ALPHA)) {

		TINYDDS_CHK_DDSFORMAT(1, 0x1, 0x0, 0, 0, TDDS_R1_UNORM);

		TINYDDS_CHK_DDSFORMAT(8, 0xF0, 0x0F, 0, 0, TDDS_G4R4_UNORM);
		TINYDDS_CHK_DDSFORMAT(8, 0x0F, 0xF0, 0, 0, TDDS_R4G4_UNORM);
		TINYDDS_CHK_DDSFORMAT(8, 0xFF, 0, 0, 0, TDDS_R8_UNORM);
		TINYDDS_CHK_DDSFORMAT(8, 0, 0, 0, 0xFF, TDDS_A8_UNORM);
		TINYDDS_CHK_DDSFORMAT(8, 0xE0, 0x1C, 0x3, 0, TDDS_B2G3R3_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0xF000, 0x0F00, 0x00F0, 0x000F, TDDS_A4B4G4R4_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0xF000, 0x0F00, 0x00F0, 0x0000, TDDS_X4B4G4R4_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0x00F0, 0x0F00, 0xF000, 0x000F, TDDS_A4R4G4B4_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0x00F0, 0x0F00, 0xF000, 0x0000, TDDS_X4R4G4B4_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0x0F00, 0x00F0, 0x000F, 0xF000, TDDS_B4G4R4A4_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0x0F00, 0x00F0, 0x000F, 0x0000, TDDS_B4G4R4X4_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0x000F, 0x00F0, 0x0F00, 0xF000, TDDS_R4G4B4A4_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0x000F, 0x00F0, 0x0F00, 0x0000, TDDS_R4G4B4X4_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0x7C00, 0x03E0, 0x001F, 0x8000, TDDS_B5G5R5A1_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0x7C00, 0x03E0, 0x001F, 0x0000, TDDS_B5G5R5X1_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0x001F, 0x03E0, 0x7C00, 0x8000, TDDS_R5G5B5A1_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0x001F, 0x03E0, 0x7C00, 0x0000, TDDS_R5G5B5X1_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0x003E, 0x07C0, 0xF800, 0x0001, TDDS_A1R5G5B5_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0x003E, 0x07C0, 0xF800, 0x0000, TDDS_X1R5G5B5_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0xF800, 0x07C0, 0x003E, 0x0001, TDDS_A1B5G5R5_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0xF800, 0x07C0, 0x003E, 0x0000, TDDS_X1B5G5R5_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0xF800, 0x07E0, 0x001F, 0x0000, TDDS_B5G6R5_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0x001F, 0x07E0, 0xF800, 0x0000, TDDS_R5G6B5_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0x00FF, 0xFF00, 0x0000, 0x0000, TDDS_R8G8_UNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0xFF00, 0x00FF, 0x0000, 0x0000, TDDS_G8R8_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0xFFFF, 0x0000, 0x0000, 0x0000, TDDS_R16_UNORM);

		TINYDDS_CHK_DDSFORMAT(16, 0xE0, 0x1C, 0x3, 0xFF00, TDDS_B2G3R3A8_UNORM);

		TINYDDS_CHK_DDSFORMAT(24, 0xFF0000, 0x00FF00, 0x0000FF, 0x0, TDDS_B8G8R8_UNORM);
		TINYDDS_CHK_DDSFORMAT(24, 0x0000FF, 0x00FF00, 0xFF0000, 0x0, TDDS_R8G8B8_UNORM);

		TINYDDS_CHK_DDSFORMAT(32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000, TDDS_R8G8B8A8_UNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000, TDDS_R8G8B8X8_UNORM);

		TINYDDS_CHK_DDSFORMAT(32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000, TDDS_B8G8R8A8_UNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000, TDDS_B8G8R8X8_UNORM);

		TINYDDS_CHK_DDSFORMAT(32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, TDDS_A8B8G8R8_UNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x00000000, TDDS_X8B8G8R8_UNORM);

		TINYDDS_CHK_DDSFORMAT(32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF, TDDS_A8R8G8B8_UNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x00000000, TDDS_X8R8G8B8_UNORM);

		TINYDDS_CHK_DDSFORMAT(32, 0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000, TDDS_R10G10B10A2_UNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0xFFC00000, 0x003FF000, 0x00000FFC, 0x00000003, TDDS_A2B10G10R10_UNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x00000FFC, 0x003FF000, 0xFFC00000, 0x00000003, TDDS_A2R10G10B10_UNORM);

		// this is often written incorrectly so we use the most 'common' version
		TINYDDS_CHK_DDSFORMAT(32, 0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000, TDDS_B10G10R10A2_UNORM);


		TINYDDS_CHK_DDSFORMAT(32, 0xFFFF0000, 0x0000FFFF, 0x00000000, 0x00000000, TDDS_G16R16_UNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000, TDDS_R16G16_UNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, TDDS_R32_UINT);

		if (ctx->header.formatRGBBitCount == 8) return TDDS_R8_UINT;
		if (ctx->header.formatRGBBitCount == 16) return TDDS_R16_UINT;
		if (ctx->header.formatRGBBitCount == 32) return TDDS_R32_UINT;
	}

	if ((ctx->header.formatFlags & TINYDDS_DDPF_BUMPDUDV) ||
			(ctx->header.formatFlags & TINYDDS_DDPF_BUMPLUMINANCE)) {
		TINYDDS_CHK_DDSFORMAT(16, 0xFF00, 0x00FF, 0x0000, 0x0000, TDDS_G8R8_SNORM);
		TINYDDS_CHK_DDSFORMAT(16, 0x00FF, 0xFF00, 0x0000, 0x0000, TDDS_R8G8_SNORM);

		TINYDDS_CHK_DDSFORMAT(32, 0xFFFF0000, 0x0000FFFF, 0x0000, 0x0, TDDS_G16R16_SNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x0000FFFF, 0xFFFF0000, 0x0000, 0x0, TDDS_R16G16_SNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000, TDDS_R8G8B8A8_SNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000, TDDS_R10G10B10A2_SNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000, TDDS_B10G10R10A2_SNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0x00000FFC, 0x003FF000, 0xFFC00000, 0x00000003, TDDS_A2R10G10B10_SNORM);
		TINYDDS_CHK_DDSFORMAT(32, 0xFFC00000, 0x003FF000, 0x00000FFC, 0x00000003, TDDS_A2B10G10R10_SNORM);

		if (ctx->header.formatRGBBitCount == 8) return TDDS_R8_SINT;
		if (ctx->header.formatRGBBitCount == 16) return TDDS_R16_SINT;
		if (ctx->header.formatRGBBitCount == 32) return TDDS_R32_SINT;
	}

	if (ctx->header.formatFlags & TINYDDS_DDPF_LUMINANCE) {
		TINYDDS_CHK_DDSFORMAT(8, 0x0F, 0x00, 0x00, 0xF0, TDDS_R4G4_UNORM); // this is A4L4 aka A4R4 we decode this as R4G4
		TINYDDS_CHK_DDSFORMAT(16, 0x00FF, 0x0000, 0x0000, 0xFF00, TDDS_R8G8_UNORM); // this is A8L8 aka A4R8 we decode this as R8G8

		if (ctx->header.formatRGBBitCount == 8) return TDDS_R8_UNORM;
		if (ctx->header.formatRGBBitCount == 16) return TDDS_R16_UNORM;
		if (ctx->header.formatRGBBitCount == 32) return TDDS_R32_UINT;

	}

	return TDDS_UNDEFINED;
}
#undef TINYDDS_CHK_DDSFORMAT

static uint32_t TinyDDS_MipMapReduce(uint32_t value, uint32_t mipmaplevel) {

	// handle 0 being passed in
	if (value <= 1)
		return 1;

	// there are better ways of doing this (log2 etc.) but this doesn't require any
	// dependecies and isn't used enough to matter imho
	for (uint32_t i = 0u; i < mipmaplevel; ++i) {
		if (value <= 1)
			return 1;
		value = value / 2;
	}
	return value;
}

bool TinyDDS_ReadHeader(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return false;

	ctx->headerPos = ctx->callbacks.tellFn(ctx->user);
	if( ctx->callbacks.readFn(ctx->user, &ctx->header, sizeof(TinyDDS_Header)) != sizeof(TinyDDS_Header)) {
		ctx->callbacks.errorFn(ctx->user, "Could not read DDS header");
		return false;
	}

	// try the easy case of a modern dx10 DDS file
	if ((ctx->header.formatFlags & TINYDDS_DDPF_FOURCC) &&
			(ctx->header.formatFourCC == TINYDDS_MAKE_RIFFCODE('D', 'X', '1', '0'))) {
		ctx->callbacks.readFn(ctx->user, &ctx->headerDx10, sizeof(TinyDDS_HeaderDX10));

		if (ctx->headerDx10.DXGIFormat >= TDDS_SYNTHESISED_DXGIFORMATS) {
			ctx->callbacks.errorFn(ctx->user, "DX10 Header has an invalid DXGI_FORMAT code");
			return false;
		}
	}

	ctx->format = TinyDDS_DecodeFormat(ctx);
	if (ctx->format == TDDS_UNDEFINED) {
		ctx->callbacks.errorFn(ctx->user, "Could not decode DDS format");
		return false;
	}

	if(	(ctx->header.formatFourCC == 0) &&
			(ctx->header.formatRGBBitCount != 0) &&
			((ctx->header.formatRGBBitCount/8) != TinyDDS_FormatSize(ctx->format))) {
		ctx->callbacks.errorFn(ctx->user, "Format size mismatch");
		return false;
	}

	// correct for dodgy mipmap levels counts
	if(ctx->header.mipMapCount > 1) {
		uint32_t w = ctx->header.width;
		uint32_t h = ctx->header.height;

		for(uint32_t i = 0; i < ctx->header.mipMapCount;++i) {
			if (TinyDDS_IsCompressed(ctx->format)) {
				if (w <= 4 || h <= 4) {
					ctx->header.mipMapCount = i + 1;
					break;
				}
			} else if (w <= 1 || h <= 1) {
				ctx->header.mipMapCount = i + 1;
				break;
			}


			w = w / 2;
			h = h / 2;
		}

	}

	if (TinyDDS_IsCompressed(ctx->format)) {
		// compressed images never get asked to make mip maps which is good as
		// requires decompress/compress cycle
		if(ctx->header.mipMapCount == 0) ctx->header.mipMapCount = 1;
	}

	if(TinyDDS_IsCLUT(ctx->format)) {
		// for now don't ask to generate mipmaps for cluts
		if(ctx->header.mipMapCount == 0) ctx->header.mipMapCount = 1;

		size_t const clutSize = 256 * sizeof(uint32_t);

		ctx->clut = (uint32_t*) ctx->callbacks.allocFn(ctx->user, clutSize);

		if( ctx->callbacks.readFn(ctx->user, (void*)ctx->clut, clutSize) != clutSize) {
			ctx->callbacks.errorFn(ctx->user, "Could not read DDS CLUT");
			return false;
		}
	}

	ctx->firstImagePos = ctx->callbacks.tellFn(ctx->user);
	ctx->headerValid = true;
	return true;
}

bool TinyDDS_IsCubemap(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return false;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return false;
	}

	return (ctx->header.caps2 & TINYDDS_DDSCAPS2_CUBEMAP);
}

bool TinyDDS_Dimensions(TinyDDS_ContextHandle handle,
												uint32_t *width,
												uint32_t *height,
												uint32_t *depth,
												uint32_t *slices) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return false;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return false;
	}

	if (width)
		*width = ctx->header.width;
	if (height)
		*height = ctx->header.height;
	if (depth)
		*depth = ctx->header.depth;
	if (slices)
		*slices = ctx->headerDx10.arraySize;
	return true;
}

uint32_t TinyDDS_Width(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return 0;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return 0;
	}
	return ctx->header.width;
}

uint32_t TinyDDS_Height(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return 0;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return 0;
	}
	return ctx->header.height;
}

uint32_t TinyDDS_Depth(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return 0;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return 0;
	}

	return ctx->header.depth;
}

uint32_t TinyDDS_ArraySlices(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return 0;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return 0;
	}

	return ctx->headerDx10.arraySize;
}

bool TinyDDS_Is1D(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return false;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return false;
	}
	return (ctx->header.height <= 1 && ctx->header.depth <= 1);
}
bool TinyDDS_Is2D(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return false;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return false;
	}
	return (ctx->header.height > 1 && ctx->header.depth <= 1);
}
bool TinyDDS_Is3D(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return false;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return false;
	}

	return (ctx->header.height > 1 && ctx->header.depth > 1);
}

bool TinyDDS_IsArray(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return false;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return false;
	}

	return (ctx->headerDx10.arraySize >= 1);
}

uint32_t TinyDDS_NumberOfMipmaps(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return 0;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return 0;
	}

	return ctx->header.mipMapCount ? ctx->header.mipMapCount : 1;
}

bool TinyDDS_NeedsGenerationOfMipmaps(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return false;
	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return false;
	}

	return ctx->header.mipMapCount == 0;
}

bool TinyDDS_NeedsEndianCorrecting(TinyDDS_ContextHandle handle) {
	// TODO should return true if this file is compiled on big endian machines
	BASISU_NOTE_UNUSED(handle);
	return false;
}

uint32_t TinyDDS_FaceSize(TinyDDS_ContextHandle handle, uint32_t mipmaplevel) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return 0;

	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return 0;
	}
	uint32_t w = TinyDDS_MipMapReduce(ctx->header.width, mipmaplevel);
	uint32_t h = TinyDDS_MipMapReduce(ctx->header.height, mipmaplevel);
	uint32_t d = TinyDDS_MipMapReduce(ctx->header.depth, mipmaplevel);
	uint32_t s = ctx->headerDx10.arraySize ? ctx->headerDx10.arraySize : 1;

	if(d > 1 && s > 1) {
		ctx->callbacks.errorFn(ctx->user, "Volume textures can't have array slices or be cubemap");
		return 0;
	}

	if (TinyDDS_IsCompressed(ctx->format)) {
		// padd to block boundaries
		w = (w + 3) / 4;
		h = (h + 3) / 4;
	}
	// 1 bit special case
	if(ctx->format == TDDS_R1_UNORM) {
		w = (w + 7) / 8;
	}

	uint32_t const formatSize = TinyDDS_FormatSize(ctx->format);
	return w * h * d * s * formatSize;
}

uint32_t TinyDDS_ImageSize(TinyDDS_ContextHandle handle, uint32_t mipmaplevel) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return 0;

	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return 0;
	}

	if(	ctx->header.caps2 & TINYDDS_DDSCAPS2_CUBEMAP ||
			ctx->headerDx10.miscFlag & TINYDDS_D3D10_RESOURCE_MISC_TEXTURECUBE ) {
		return TinyDDS_FaceSize(handle, mipmaplevel) * 6;
	} else {
		return TinyDDS_FaceSize(handle, mipmaplevel);
	}
}

void const *TinyDDS_ImageRawData(TinyDDS_ContextHandle handle, uint32_t mipmaplevel) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return NULL;

	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return NULL;
	}

	if (mipmaplevel >= (ctx->header.mipMapCount ? ctx->header.mipMapCount : 1) ) {
		ctx->callbacks.errorFn(ctx->user, "Invalid mipmap level");
		return NULL;
	}

	if (mipmaplevel >= TINYDDS_MAX_MIPMAPLEVELS) {
		ctx->callbacks.errorFn(ctx->user, "Invalid mipmap level");
		return NULL;
	}

	if (ctx->mipmaps[mipmaplevel] != NULL)
		return ctx->mipmaps[mipmaplevel];

	if(	ctx->header.caps2 & TINYDDS_DDSCAPS2_CUBEMAP ||
			ctx->headerDx10.miscFlag & TINYDDS_D3D10_RESOURCE_MISC_TEXTURECUBE ) {

		uint64_t offset = 0;
		for(uint32_t i=0;i < mipmaplevel;++i) {
			offset += TinyDDS_FaceSize(handle, i);
		}

		uint32_t mipMapCount = ctx->header.mipMapCount;
		if(mipMapCount == 0) mipMapCount = 1;

		// at least one cubemap generater has mipMapCount wrong which causes
		// image artifacts :(
		uint64_t nextFaceOffset = 0;
		for(uint32_t i = 0;i < mipMapCount;++i) {
			nextFaceOffset += TinyDDS_FaceSize(handle, i);
		}

		size_t const faceSize = TinyDDS_FaceSize(handle, mipmaplevel);
		ctx->mipmaps[mipmaplevel] = (uint8_t const *) ctx->callbacks.allocFn(ctx->user, faceSize * 6);
		if(!ctx->mipmaps[mipmaplevel]) return NULL;

		uint8_t *dstPtr = (uint8_t*)ctx->mipmaps[mipmaplevel];
		for (uint32_t i = 0u;i < 6;++i) {
			ctx->callbacks.seekFn(ctx->user, offset + ctx->firstImagePos);
			size_t read = ctx->callbacks.readFn(ctx->user, (void *) dstPtr, faceSize);
			if(read != faceSize) {
				ctx->callbacks.freeFn(ctx->user, (void*)&ctx->mipmaps[mipmaplevel]);
				return NULL;
			}
			dstPtr += faceSize;
			offset += nextFaceOffset;
		}
		return ctx->mipmaps[mipmaplevel];
	}

	uint64_t offset = 0;
	for(uint32_t i=0;i < mipmaplevel;++i) {
		offset += TinyDDS_ImageSize(handle, i);
	}

	uint32_t size = TinyDDS_ImageSize(handle, mipmaplevel);
	if (size == 0)
		return NULL;

	ctx->callbacks.seekFn(ctx->user, offset + ctx->firstImagePos);

	ctx->mipmaps[mipmaplevel] = (uint8_t const *) ctx->callbacks.allocFn(ctx->user, size);
	if (!ctx->mipmaps[mipmaplevel]) return NULL;
	size_t read = ctx->callbacks.readFn(ctx->user, (void *) ctx->mipmaps[mipmaplevel], size);
	if(read != size) {
		ctx->callbacks.freeFn(ctx->user, (void*)&ctx->mipmaps[mipmaplevel]);
		return NULL;
	}

	return ctx->mipmaps[mipmaplevel];
}

TinyDDS_Format TinyDDS_GetFormat(TinyDDS_ContextHandle handle) {
	TinyDDS_Context *ctx = (TinyDDS_Context *) handle;
	if (ctx == NULL)
		return TDDS_UNDEFINED;

	if (!ctx->headerValid) {
		ctx->callbacks.errorFn(ctx->user, "Header data hasn't been read yet or its invalid");
		return TDDS_UNDEFINED;
	}
	return ctx->format;
}

#define TDDS_EF(bits, rm, gm, bm, am, fl) \
		header->formatRGBBitCount = bits; \
		header->formatRBitMask = rm; \
		header->formatGBitMask = gm; \
		header->formatBBitMask = bm; \
		header->formatABitMask = am; \
		header->formatFlags = fl; \
		header->formatFourCC = 0; \
		return true;

#define TDDS_EF_RGB(bits, rm, gm, bm) TDDS_EF(bits, rm, gm, bm, 0, TINYDDS_DDPF_RGB )
#define TDDS_EF_RGBA(bits, rm, gm, bm, am) TDDS_EF(bits, rm, gm, bm, am, TINYDDS_DDPF_RGB | TINYDDS_DDPF_ALPHAPIXELS)
#define TDDS_EF_ALPHA(bits, am) TDDS_EF(bits, 0, 0, 0, am, TINYDDS_DDPF_ALPHA)

#define TDDS_EF_BUMP_RG(bits, rm, gm) TDDS_EF(bits, rm, gm, 0, 0, TINYDDS_DDPF_BUMPDUDV)
#define TDDS_EF_BUMP_RGB(bits, rm, gm, bm) TDDS_EF(bits, rm, gm, bm, 0, TINYDDS_DDPF_BUMPLUMINANCE)
#define TDDS_EF_BUMP_RGBA(bits, rm, gm, bm, am) TDDS_EF(bits, rm, gm, bm, am, TINYDDS_DDPF_BUMPLUMINANCE | TINYDDS_DDPF_ALPHAPIXELS)

static bool TinyDDS_EncodeFormat(TinyDDS_Format fmt, TinyDDS_Header* header, TinyDDS_HeaderDX10* headerDx10) {
	// lets start with the easy part. if its real DXGI_FORMAT we can just fill in the Dx10 part
	if(fmt < TDDS_SYNTHESISED_DXGIFORMATS) {
		headerDx10->DXGIFormat = (TinyImageFormat_DXGI_FORMAT)fmt;
		header->formatFourCC = TINYDDS_MAKE_RIFFCODE('D','X','1','0');
		header->formatFlags = TINYDDS_DDPF_FOURCC;
	} else {
		headerDx10->DXGIFormat = TIF_DXGI_FORMAT_UNKNOWN;
	}
	// now lets try synthesising if possible
	// if we can reset the DX10 fourCC but leave the format in place
	// that way if we have slices which can only be DXGI_FORMAT we can use it
	switch(fmt) {
	case TDDS_UNDEFINED: break;

	case TDDS_R1_UNORM: TDDS_EF_RGB(1, 0x1, 0, 0)
	case TDDS_R4G4_UNORM: TDDS_EF_RGB(8, 0x0F, 0xF0, 0)
	case TDDS_G4R4_UNORM: TDDS_EF_RGB(8, 0xF0, 0x0F, 0)
	case TDDS_B2G3R3_UNORM: TDDS_EF_RGB(8, 0x3, 0x7, 0x7 )
	case TDDS_R8_UNORM: TDDS_EF_RGB(8, 0xFF, 0, 0 );
	case TDDS_A8_UNORM: TDDS_EF_ALPHA( 8, 0xFF);

	case TDDS_R16_UNORM:TDDS_EF_RGB( 16,0x0000FFFF, 0, 0)
	case TDDS_A4B4G4R4_UNORM:
		TDDS_EF_RGBA(16, 0xF000, 0x0F00, 0x00F0, 0x000F);
	case TDDS_X4B4G4R4_UNORM:
		TDDS_EF_RGBA(16, 0xF000, 0x0F00, 0x00F0, 0x0000);
	case TDDS_B4G4R4A4_UNORM:
		TDDS_EF_RGBA(16, 0x0F00, 0x00F0, 0x000F, 0xF000);
	case TDDS_B4G4R4X4_UNORM:
		TDDS_EF_RGBA(16, 0x0F00, 0x00F0, 0x000F, 0x0000);
	case TDDS_A4R4G4B4_UNORM:
		TDDS_EF_RGBA(16, 0x00F0, 0x0F00, 0xF000, 0x000F);
	case TDDS_X4R4G4B4_UNORM:
		TDDS_EF_RGBA(16, 0x00F0, 0x0F00, 0xF000, 0x0000);
	case TDDS_R4G4B4A4_UNORM:
		TDDS_EF_RGBA(16, 0x000F, 0x00F0, 0x0F00, 0xF000);
	case TDDS_R4G4B4X4_UNORM:
		TDDS_EF_RGBA(16, 0x000F, 0x00F0, 0x0F00, 0x0000);

	case TDDS_B5G5R5A1_UNORM:
		TDDS_EF_RGBA(16, 0x7C00, 0x03E0, 0x001F, 0x8000);
	case TDDS_B5G5R5X1_UNORM:
		TDDS_EF_RGBA(16, 0x7C00, 0x03E0, 0x001F, 0x0000);

	case TDDS_R5G5B5A1_UNORM:
	TDDS_EF_RGBA(16, 0x001F, 0x03E0, 0x7C00, 0x8000);
	case TDDS_R5G5B5X1_UNORM:
	TDDS_EF_RGBA(16, 0x001F, 0x03E0, 0x7C00, 0x0000);

	case TDDS_A1R5G5B5_UNORM:
		TDDS_EF_RGBA(16, 0x003E, 0x07C0, 0xF800, 0x0001);
	case TDDS_X1R5G5B5_UNORM:
		TDDS_EF_RGBA(16, 0x003E, 0x07C0, 0xF800, 0x0000);
	case TDDS_A1B5G5R5_UNORM:
		TDDS_EF_RGBA(16, 0xF800, 0x07C0, 0x003E, 0x0001);
	case TDDS_X1B5G5R5_UNORM:
		TDDS_EF_RGBA(16, 0xF800, 0x07C0, 0x003E, 0x0000);

	case TDDS_B5G6R5_UNORM:
		TDDS_EF_RGB(16, 0xF800, 0x07E0, 0x001F);
	case TDDS_R5G6B5_UNORM:
		TDDS_EF_RGB(16, 0x001F, 0x07E0, 0xF800);

	case TDDS_R8G8_UNORM:
		TDDS_EF_RGB(16, 0x00FF, 0xFF00, 0);
	case TDDS_G8R8_UNORM:
		TDDS_EF_RGB(16, 0xFF00, 0x00FF, 0);
	case TDDS_G8R8_SNORM:
		TDDS_EF_BUMP_RG(16, 0xFF00, 0x00FF);

	case TDDS_B2G3R3A8_UNORM: TDDS_EF_RGBA(8, 0x3, 0x7, 0x7, 0xFF00 )

	case TDDS_R8G8B8_UNORM:
	TDDS_EF_RGB( 24,0x000000FF, 0x0000FF00, 0x00FF0000)
	case TDDS_B8G8R8_UNORM:
	TDDS_EF_RGB( 24,0x00FF0000, 0x0000FF00, 0x000000FF)

	case TDDS_R8G8B8A8_UNORM:
	TDDS_EF_RGBA( 32,0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000)
	case TDDS_R8G8B8X8_UNORM:
	TDDS_EF_RGBA( 32,0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000)
	case TDDS_B8G8R8A8_UNORM:
	TDDS_EF_RGBA( 32,0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000)
	case TDDS_B8G8R8X8_UNORM:
	TDDS_EF_RGBA( 32,0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000)
	case TDDS_A8B8G8R8_UNORM:
	TDDS_EF_RGBA( 32,0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF)
	case TDDS_X8B8G8R8_UNORM:
	TDDS_EF_RGBA( 32,0xFF000000, 0x00FF0000, 0x0000FF00, 0x00000000)
	case TDDS_A8R8G8B8_UNORM:
	TDDS_EF_RGBA( 32,0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF)
	case TDDS_X8R8G8B8_UNORM:
	TDDS_EF_RGBA( 32,0x0000FF00, 0x00FF0000, 0xFF000000, 0x00000000)

	/* A2R10G10B10 is broken via the traditional DDS descriptions, so we
	 * always use the Dx10 header for those
	case TDDS_R10G10B10A2_UNORM:
	TDDS_EF_RGBA( 32,0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000)
	case TDDS_A2B10G10R10_UNORM:
	TDDS_EF_RGBA( 32,0xFFC00000, 0x003FF000, 0x00000FFC, 0x00000003)
	case TDDS_A2R10G10B10_UNORM:
	TDDS_EF_RGBA( 32,0x00000FFC, 0x003FF000, 0xFFC00000, 0x00000003)
	case TDDS_B10G10R10A2_UNORM:
	TDDS_EF_RGBA( 32,0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000)
	*/
	case TDDS_R10G10B10A2_UNORM:
	case TDDS_B10G10R10A2_UNORM:
	case TDDS_A2B10G10R10_UNORM:
	case TDDS_A2R10G10B10_UNORM:
	case TDDS_R10G10B10A2_SNORM:
	case TDDS_B10G10R10A2_SNORM:
	case TDDS_A2B10G10R10_SNORM:
	case TDDS_A2R10G10B10_SNORM:
		break;

	case TDDS_R16G16_UNORM: TDDS_EF_RGB( 32,0x0000FFFF, 0xFFFF0000, 0)
	case TDDS_G16R16_UNORM: TDDS_EF_RGB( 32,0xFFFF0000, 0x0000FFFF, 0)

	case TDDS_BC1_RGBA_UNORM_BLOCK:
		header->formatFourCC = TINYDDS_MAKE_RIFFCODE('D','X','T','1');
		header->formatFlags = TINYDDS_DDPF_FOURCC;
		return true;
	case TDDS_BC2_UNORM_BLOCK:
		header->formatFourCC = TINYDDS_MAKE_RIFFCODE('D','X','T','3');
		header->formatFlags = TINYDDS_DDPF_FOURCC;
		return true;
	case TDDS_BC3_UNORM_BLOCK:
		header->formatFourCC = TINYDDS_MAKE_RIFFCODE('D','X','T','5');
		header->formatFlags = TINYDDS_DDPF_FOURCC;
		return true;
	case TDDS_BC4_UNORM_BLOCK:
		header->formatFourCC = TINYDDS_MAKE_RIFFCODE('A','T','I','1');
		header->formatFlags = TINYDDS_DDPF_FOURCC;
		return true;
	case TDDS_BC5_UNORM_BLOCK:
		header->formatFourCC = TINYDDS_MAKE_RIFFCODE('A','T','I','2');
		header->formatFlags = TINYDDS_DDPF_FOURCC;
		return true;


	case TDDS_R8_SNORM:
	case TDDS_R8G8_SNORM:
	case TDDS_R8G8B8A8_SNORM:
	case TDDS_R16_SNORM:
	case TDDS_R16G16_SNORM:
	case TDDS_A8B8G8R8_SNORM:
	case TDDS_B8G8R8A8_SNORM:
	case TDDS_G16R16_SNORM:

	case TDDS_R8_UINT:
	case TDDS_R8_SINT:
	case TDDS_R8G8_UINT:
	case TDDS_R8G8_SINT:
	case TDDS_R8G8B8A8_UINT:
	case TDDS_R8G8B8A8_SINT:
	case TDDS_R8G8B8A8_SRGB:
	case TDDS_B8G8R8A8_SRGB:
	case TDDS_R9G9B9E5_UFLOAT:
	case TDDS_R10G10B10A2_UINT:
	case TDDS_R11G11B10_UFLOAT:
	case TDDS_R16_UINT:
	case TDDS_R16_SINT:
	case TDDS_R16_SFLOAT:
	case TDDS_R16G16_UINT:
	case TDDS_R16G16_SINT:
	case TDDS_R16G16_SFLOAT:
	case TDDS_R16G16B16A16_UNORM:
	case TDDS_R16G16B16A16_SNORM:
	case TDDS_R16G16B16A16_UINT:
	case TDDS_R16G16B16A16_SINT:
	case TDDS_R16G16B16A16_SFLOAT:
	case TDDS_R32_UINT:
	case TDDS_R32_SINT:
	case TDDS_R32_SFLOAT:
	case TDDS_R32G32_UINT:
	case TDDS_R32G32_SINT:
	case TDDS_R32G32_SFLOAT:
	case TDDS_R32G32B32_UINT:
	case TDDS_R32G32B32_SINT:
	case TDDS_R32G32B32_SFLOAT:
	case TDDS_R32G32B32A32_UINT:
	case TDDS_R32G32B32A32_SINT:
	case TDDS_R32G32B32A32_SFLOAT:
	case TDDS_BC1_RGBA_SRGB_BLOCK:
	case TDDS_BC2_SRGB_BLOCK:
	case TDDS_BC3_SRGB_BLOCK:
	case TDDS_BC4_SNORM_BLOCK:
	case TDDS_BC5_SNORM_BLOCK:
	case TDDS_BC6H_UFLOAT_BLOCK:
	case TDDS_BC6H_SFLOAT_BLOCK:
	case TDDS_BC7_UNORM_BLOCK:
	case TDDS_BC7_SRGB_BLOCK:
	case TDDS_AYUV:
	case TDDS_Y410:
	case TDDS_Y416:
	case TDDS_NV12:
	case TDDS_P010:
	case TDDS_P016:
	case TDDS_420_OPAQUE:
	case TDDS_YUY2:
	case TDDS_Y210:
	case TDDS_Y216:
	case TDDS_NV11:
	case TDDS_AI44:
	case TDDS_IA44:
	case TDDS_P8:
	case TDDS_A8P8:
	case TDDS_R10G10B10_7E3_A2_FLOAT:
	case TDDS_R10G10B10_6E4_A2_FLOAT:
	case TDDS_D16_UNORM_S8_UINT:
	case TDDS_R16_UNORM_X8_TYPELESS:
	case TDDS_X16_TYPELESS_G8_UINT:
	case TDDS_P208:
	case TDDS_V208:
	case TDDS_V408:
	case TDDS_R10G10B10_SNORM_A2_UNORM:
		break;

	}
	// these formats can probably be done via dx10 header so check
	if(headerDx10->DXGIFormat == TIF_DXGI_FORMAT_UNKNOWN) return false;
	else return true;
}

#undef TDDS_EF
#undef TDDS_EF_RGB
#undef TDDS_EF_RGBA
#undef TDDS_EF_ALPHA

bool TinyDDS_WriteImage(TinyDDS_WriteCallbacks const *callbacks,
												void *user,
												uint32_t width,
												uint32_t height,
												uint32_t depth,	 // 3D texture depth
												uint32_t slices, // Array slices
												uint32_t mipmaplevels,
												TinyDDS_Format format,
												bool cubemap,
												bool preferDx10Format,
												uint32_t const *mipmapsizes,
												void const **mipmaps) {
	TinyDDS_Header header;
	TinyDDS_HeaderDX10 headerDX10;
	memset(&header, 0, sizeof(header));
	memset(&headerDX10, 0, sizeof(headerDX10));

	header.magic = TINYDDS_MAKE_RIFFCODE('D', 'D', 'S', ' ');
	header.size = 124;
	header.formatSize = 32;

	header.width = width;
	header.height = height;
	header.depth = (depth > 1) ? depth : 0;
	header.mipMapCount = mipmaplevels;

	if(!TinyDDS_EncodeFormat(format, &header, &headerDX10)) return false;

	// do we have to force dx10 (for slices)
	if (slices > 1) {
		if(headerDX10.DXGIFormat == TIF_DXGI_FORMAT_UNKNOWN) {
			// DDS doesn't support slices for formats that aren't DXGI compatible
			return false;
		}
		header.formatFlags = TINYDDS_DDPF_FOURCC;
		header.formatFourCC = TINYDDS_MAKE_RIFFCODE('D','X','1','0');
		headerDX10.arraySize = slices;
	}
	header.flags = TINYDDS_DDSD_CAPS | TINYDDS_DDSD_PIXELFORMAT | TINYDDS_DDSD_MIPMAPCOUNT;
	header.caps1 = TINYDDS_DDSCAPS_TEXTURE | TINYDDS_DDSCAPS_COMPLEX | TINYDDS_DDSCAPS_MIPMAP;

	if(depth > 1) {
		headerDX10.resourceDimension = TINYDDS_D3D10_RESOURCE_DIMENSION_TEXTURE3D;
		header.flags |= TINYDDS_DDSD_DEPTH;
		header.caps2 |= TINYDDS_DDSCAPS2_VOLUME;
	}
	else if(height > 1) {
		headerDX10.resourceDimension = TINYDDS_D3D10_RESOURCE_DIMENSION_TEXTURE2D;
		header.flags |= TINYDDS_DDSD_HEIGHT;
	}
	else if(width > 1) {
		headerDX10.resourceDimension = TINYDDS_D3D10_RESOURCE_DIMENSION_TEXTURE1D;
		header.flags |= TINYDDS_DDSD_WIDTH;
	}
	if(cubemap) {
		headerDX10.miscFlag |= TINYDDS_D3D10_RESOURCE_MISC_TEXTURECUBE;
		header.caps2 |= TINYDDS_DDSCAPS2_CUBEMAP | TINYDDS_DDSCAPS2_CUBEMAP_ALL;
	}

	// unclear whether we need to save this or exactly what it should be...
	header.pitchOrLinearSize = 0;
	if(preferDx10Format && headerDX10.DXGIFormat != TIF_DXGI_FORMAT_UNKNOWN) {
		header.formatFlags = TINYDDS_DDPF_FOURCC;
		header.formatFourCC = TINYDDS_MAKE_RIFFCODE('D','X','1','0');
	}

	// now write
	callbacks->write(user, &header, sizeof(TinyDDS_Header));
	if(header.formatFlags & TINYDDS_DDPF_FOURCC &&
			header.formatFourCC == TINYDDS_MAKE_RIFFCODE('D','X','1','0')) {
		callbacks->write(user, &headerDX10, sizeof(TinyDDS_HeaderDX10));
	}

	// rg 8/27/2024: The original tinydds.h code is wrong for mipmapped cubemaps.
	// I'm going to work around this by having the caller compose the top mip data correctly.
	// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-file-layout-for-cubic-environment-maps
	for (uint32_t mipMapLevel = 0; mipMapLevel < header.mipMapCount; mipMapLevel++)
	{
		// rg: Adding this check, in case the caller wants to compose all the data themselves.
		if (mipmapsizes[mipMapLevel])
		{
			callbacks->write(user, mipmaps[mipMapLevel], mipmapsizes[mipMapLevel]);
		}
	}
	return true;
}

#endif

#ifdef __cplusplus
};
#endif

#endif // end header
/*
MIT License

Copyright (c) 2019 DeanoC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
