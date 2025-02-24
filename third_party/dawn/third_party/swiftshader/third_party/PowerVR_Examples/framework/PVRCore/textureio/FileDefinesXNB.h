/*!
\brief Defines used internally by the XNB reader.
\file PVRCore/textureio/FileDefinesXNB.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <cstdint>
namespace pvr {
namespace texture_xnb {
/// <summary>File header format for XNB files.</summary>
struct FileHeader
{
	uint8_t identifier[3]; //!< Identifier used for XNB files
	uint8_t platform; //!< The platform used
	uint8_t version; //!< The version of the XNB file
	uint8_t flags; //!< Flags used
	uint32_t fileSize; //!< The size of the file
};

/// <summary>Texture 2D header for XNB files.</summary>
struct Texture2DHeader
{
	int32_t format; //!< The format used
	uint32_t width; //!< The width ofthe image
	uint32_t height; //!< The height of the image
	uint32_t numMipMaps; //!< The number of mip maps used
};

/// <summary>Texture 3D header for XNB files.</summary>
struct Texture3DHeader
{
	int32_t format; //!< The format used
	uint32_t width; //!< The width of the image
	uint32_t height; //!< The height of the image
	uint32_t depth; //!< The depth of the image
	uint32_t numMipMaps; //!< The number of mip maps used
};

/// <summary>Texture Cube header for XNB files.</summary>
struct TextureCubeHeader
{
	int32_t format; //!< The format used
	uint32_t size; //!< The size of the image
	uint32_t numMipMaps; //!< The number of mip maps used
};

/// <summary>Flags used for XNB files.</summary>
enum Flags
{
	e_fileCompressed = 0x80
};

/// <summary>Pixel formats used for XNB files.</summary>
enum PixelFormat
{
	FormatRGBA,
	FormatBGR565,
	FormatBGRA5551,
	FormatBGRA4444,
	FormatDXT1,
	FormatDXT3,
	FormatDXT5,
	FormatNormalizedByte2,
	FormatNormalizedByte4,
	FormatRGBA1010102,
	FormatRG32,
	FormatRGBA64,
	FormatAlpha8,
	FormatSingle,
	FormatVector2,
	FormatVector4,
	FormatHalfSingle,
	FormatHalfVector2,
	FormatHalfVector4,
	FormatHDRBlendable,

	NumXNBFormats
};

// Magic identifier
static const uint8_t c_identifier[] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

// Reference number to verify endianness of a file
static const uint32_t c_endianReference = 0x04030201;

// Expected size of a header in file
static const uint32_t c_expectedHeaderSize = 10;

} // namespace texture_xnb
} // namespace pvr
