/*!
\brief Defines used internally by the DDS reader.
\file PVRCore/textureio/FileDefinesDDS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <cstdint>

namespace pvr {
namespace texture_dds {
/// <summary>Pixel format used in DDS files.</summary>
struct PixelFormat
{
	uint32_t size; //!< The size of the DDS file
	uint32_t flags; //!< Flags for the DDS file
	uint32_t fourCC; //!< Meant to be four characters, but is easier to manage with a MAKEFOURCC value.
	uint32_t bitCount; //!< The bit count used in the DDS file
	uint32_t redMask; //!< A red channel mask
	uint32_t greenMask; //!< A green channel mask
	uint32_t blueMask; //!< A blue channel mask
	uint32_t alphaMask; //!< An alpha channel mask
};

/// <summary>Pixel Format flag values.</summary>
enum PixelFormatFlags
{
	e_alphaPixels = 0x00000001,
	e_alpha = 0x00000002,
	e_fourCC = 0x00000004,
	e_rgb = 0x00000040,
	e_yuv = 0x00000200,
	e_luminance = 0x00020000,
	// Neither of the below flags are specified in the programming guide, but were used by the legacy DirectX Texture Tool
	e_unknownBump1 = 0x00040000,
	e_unknownBump2 = 0x00080000
};

static const uint32_t c_expectedPixelFormatSize = 32;

/// <summary>File header format for DDS files.</summary>
struct FileHeader
{
	uint32_t size; //!< The size of the DDS File
	uint32_t flags; //!< The flags used by the DDS
	uint32_t height; //!< The height of the image
	uint32_t width; //!< The width of the image
	uint32_t pitchOrLinearSize; //!< Specifies the pitch or linear size of the image
	uint32_t depth; //!< The depth of the image
	uint32_t numMipMaps; //!< The number of mip maps
	uint32_t reserved[11]; //!< Reserved bits
	PixelFormat pixelFormat; //!< The pixel format used
	uint32_t Capabilities1; //!< Capabilities
	uint32_t Capabilities2; //!< Capabilities
	uint32_t Capabilities3; //!< Capabilities
	uint32_t Capabilities4; //!< Capabilities
	uint32_t reserved2; //!< Reserved
};

static const uint32_t c_magicIdentifier = 0x20534444; // "DDS "
static const uint32_t c_expectedDDSSize = 124;

/// <summary>DDS FileHeader flag values.</summary>
enum Flags
{
	e_Capabilities = 0x00000001,
	e_height = 0x00000002,
	e_width = 0x00000004,
	e_pitch = 0x00000008,
	e_pixelFormat = 0x00001000,
	e_numMipMaps = 0x00020000,
	e_linearSize = 0x00080000,
	e_depth = 0x00800000
};

/// <summary>Flag values in Capabilities1.</summary>
enum Capabilities1Flags
{
	e_complex = 0x00000008,
	e_texture = 0x00001000,
	e_mipMaps = 0x00400000
};

/// <summary>Flag values in Capabilities2.</summary>
enum Capabilities2Flags
{
	e_cubeMap = 0x00000200,
	e_cubeMapPositiveX = 0x00000400,
	e_cubeMapNegativeX = 0x00000800,
	e_cubeMapPositiveY = 0x00001000,
	e_cubeMapNegativeY = 0x00002000,
	e_cubeMapPositiveZ = 0x00004000,
	e_cubeMapNegativeZ = 0x00008000,
	e_volume = 0x00200000
};

/// <summary>File header for DX10.</summary>
struct FileHeaderDX10
{
	uint32_t dxgiFormat; //!< The format
	uint32_t resourceDimension; //!< The dimension of the resource
	uint32_t miscFlags; //!< See DDS_RESOURCE_MISC_FLAG
	uint32_t arraySize; //!< The array size
	uint32_t miscFlags2; //!< Flags
};

/// <summary>Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION.</summary>
enum TextureDimension
{
	e_texture1D = 2,
	e_texture2D = 3,
	e_texture3D = 4,
};

/// <summary>Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG.</summary>
enum TextureMiscellaneousFlags
{
	e_textureCube = 0x4L,
};

/// <summary>Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG.</summary>
enum TextureMiscellaneousFlags2
{
	Unknown,
	e_straight,
	e_premultiplied,
	e_opaque,
	e_custom
};

/// <summary>Make FourCC template.</summary>
template<char C1Name, char C2Name, char C3Name, char C4Name>
class MakeFourCC
{
public:
	/// <summary>The compile time generated FourCC value constructed using the template parameters.</summary>
	static const uint64_t FourCC =
		((static_cast<uint32_t>(C1Name)) + (static_cast<uint32_t>(C2Name) << 8) + (static_cast<uint32_t>(C3Name) << 16) + (static_cast<uint32_t>(C4Name) << 24));
};

/// <summary>Direct3D (Up to DirectX 9) formats.</summary>
enum D3DFormat
{
	D3DFMT_UNKNOWN = 0,

	D3DFMT_R8G8B8 = 20,
	D3DFMT_A8R8G8B8 = 21,
	D3DFMT_X8R8G8B8 = 22,
	D3DFMT_R5G6B5 = 23,
	D3DFMT_X1R5G5B5 = 24,
	D3DFMT_A1R5G5B5 = 25,
	D3DFMT_A4R4G4B4 = 26,
	D3DFMT_R3G3B2 = 27,
	D3DFMT_A8 = 28,
	D3DFMT_A8R3G3B2 = 29,
	D3DFMT_X4R4G4B4 = 30,
	D3DFMT_A2B10G10R10 = 31,
	D3DFMT_A8B8G8R8 = 32,
	D3DFMT_X8B8G8R8 = 33,
	D3DFMT_G16R16 = 34,
	D3DFMT_A2R10G10B10 = 35,
	D3DFMT_A16B16G16R16 = 36,

	D3DFMT_A8P8 = 40,
	D3DFMT_P8 = 41,

	D3DFMT_L8 = 50,
	D3DFMT_A8L8 = 51,
	D3DFMT_A4L4 = 52,

	D3DFMT_V8U8 = 60,
	D3DFMT_L6V5U5 = 61,
	D3DFMT_X8L8V8U8 = 62,
	D3DFMT_Q8W8V8U8 = 63,
	D3DFMT_V16U16 = 64,
	D3DFMT_A2W10V10U10 = 67,

	D3DFMT_L16 = 81,

	D3DFMT_Q16W16V16U16 = 110,

	D3DFMT_R16F = 111,
	D3DFMT_G16R16F = 112,
	D3DFMT_A16B16G16R16F = 113,

	D3DFMT_R32F = 114,
	D3DFMT_G32R32F = 115,
	D3DFMT_A32B32G32R32F = 116,

	D3DFMT_UYVY = MakeFourCC<'U', 'Y', 'V', 'Y'>::FourCC,
	D3DFMT_R8G8_B8G8 = MakeFourCC<'R', 'G', 'B', 'G'>::FourCC,
	D3DFMT_YUY2 = MakeFourCC<'Y', 'U', 'Y', '2'>::FourCC,
	D3DFMT_G8R8_G8B8 = MakeFourCC<'G', 'R', 'G', 'B'>::FourCC,
	D3DFMT_DXT1 = MakeFourCC<'D', 'X', 'T', '1'>::FourCC,
	D3DFMT_DXT2 = MakeFourCC<'D', 'X', 'T', '2'>::FourCC,
	D3DFMT_DXT3 = MakeFourCC<'D', 'X', 'T', '3'>::FourCC,
	D3DFMT_DXT4 = MakeFourCC<'D', 'X', 'T', '4'>::FourCC,
	D3DFMT_DXT5 = MakeFourCC<'D', 'X', 'T', '5'>::FourCC,
	D3DFMT_PVRTC2 = MakeFourCC<'P', 'T', 'C', '2'>::FourCC,
	D3DFMT_PVRTC4 = MakeFourCC<'P', 'T', 'C', '4'>::FourCC,

	D3DFMT_FORCE_DWORD = 0x7fffffff
};

/// <summary>DXGI (DirectX 10 onwards) formats.</summary>
enum DXGIFormat
{
	DXGI_FORMAT_UNKNOWN = 0,
	DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
	DXGI_FORMAT_R32G32B32A32_UINT = 3,
	DXGI_FORMAT_R32G32B32A32_SINT = 4,
	DXGI_FORMAT_R32G32B32_FLOAT = 6,
	DXGI_FORMAT_R32G32B32_UINT = 7,
	DXGI_FORMAT_R32G32B32_SINT = 8,
	DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM = 11,
	DXGI_FORMAT_R16G16B16A16_UINT = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM = 13,
	DXGI_FORMAT_R16G16B16A16_SINT = 14,
	DXGI_FORMAT_R32G32_FLOAT = 16,
	DXGI_FORMAT_R32G32_UINT = 17,
	DXGI_FORMAT_R32G32_SINT = 18,
	DXGI_FORMAT_R10G10B10A2_UNORM = 24,
	DXGI_FORMAT_R10G10B10A2_UINT = 25,
	DXGI_FORMAT_R11G11B10_FLOAT = 26,
	DXGI_FORMAT_R8G8B8A8_UNORM = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	DXGI_FORMAT_R8G8B8A8_UINT = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM = 31,
	DXGI_FORMAT_R8G8B8A8_SINT = 32,
	DXGI_FORMAT_R16G16_FLOAT = 34,
	DXGI_FORMAT_R16G16_UNORM = 35,
	DXGI_FORMAT_R16G16_UINT = 36,
	DXGI_FORMAT_R16G16_SNORM = 37,
	DXGI_FORMAT_R16G16_SINT = 38,
	DXGI_FORMAT_R32_FLOAT = 41,
	DXGI_FORMAT_R32_UINT = 42,
	DXGI_FORMAT_R32_SINT = 43,
	DXGI_FORMAT_R8G8_UNORM = 49,
	DXGI_FORMAT_R8G8_UINT = 50,
	DXGI_FORMAT_R8G8_SNORM = 51,
	DXGI_FORMAT_R8G8_SINT = 52,
	DXGI_FORMAT_R16_FLOAT = 54,
	DXGI_FORMAT_R16_UNORM = 56,
	DXGI_FORMAT_R16_UINT = 57,
	DXGI_FORMAT_R16_SNORM = 58,
	DXGI_FORMAT_R16_SINT = 59,
	DXGI_FORMAT_R8_TYPELESS = 60,
	DXGI_FORMAT_R8_UNORM = 61,
	DXGI_FORMAT_R8_UINT = 62,
	DXGI_FORMAT_R8_SNORM = 63,
	DXGI_FORMAT_R8_SINT = 64,
	DXGI_FORMAT_A8_UNORM = 65,
	DXGI_FORMAT_R1_UNORM = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
	DXGI_FORMAT_BC1_UNORM = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB = 72,
	DXGI_FORMAT_BC2_UNORM = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB = 75,
	DXGI_FORMAT_BC3_UNORM = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB = 78,
	DXGI_FORMAT_BC4_UNORM = 80,
	DXGI_FORMAT_BC4_SNORM = 81,
	DXGI_FORMAT_BC5_UNORM = 83,
	DXGI_FORMAT_BC5_SNORM = 84,
	DXGI_FORMAT_B5G6R5_UNORM = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	DXGI_FORMAT_BC6H_UF16 = 95,
	DXGI_FORMAT_BC6H_SF16 = 96,
	DXGI_FORMAT_BC7_UNORM = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB = 99,
	DXGI_FORMAT_YUY2 = 107,
	DXGI_FORMAT_AI44 = 111,
	DXGI_FORMAT_IA44 = 112,
	DXGI_FORMAT_B4G4R4A4_UNORM = 115,
	DXGI_FORMAT_FORCE_UINT = 0xffffffffUL,
};
} // namespace texture_dds
} // namespace pvr
