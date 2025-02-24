/*!
\brief Defines used internally by the BMP reader.
\file PVRCore/textureio/FileDefinesBMP.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <cstdint>
namespace pvr {
namespace texture_bmp {
/// <summary>Structure of a file header for a BMP file</summary>
struct FileHeader
{
	uint16_t signature; //!< Signature
	uint32_t fileSize; //!< Size of the BMP file
	uint16_t reserved1; //!< Reserved
	uint16_t reserved2; //!< Reserved
	uint32_t pixelOffset; //!< Pixel offset
};

/// <summary>Structure of a core header for a BMP file</summary>
struct CoreHeader
{
	uint32_t headerSize; //!< Size of the header
	uint16_t width; //!< Width of the BMP
	uint16_t height; //!< Height of the BMP
	uint16_t numPlanes; //!< Number of planes in the BMP
	uint16_t bitsPerPixel; //!< Bits per pixel used by the BMP
};

/// <summary>Structure of a info header1 for a BMP file</summary>
struct InfoHeader1
{
	uint32_t headerSize; //!< Size of the header
	int32_t width; //!< Width of the BMP
	int32_t height; //!< Height of the BMP
	uint16_t numPlanes; //!< Number of planes in the BMP
	uint16_t bitsPerPixel; //!< Bits per pixel used by the BMP
	uint32_t compressionType; //!< Compression type used
	uint32_t imageSize; //!< Size of the image
	int32_t horizontalPixelsPerMeter; //!< The number of pixels per meter in the horizontal direction
	int32_t verticalPixelsPerMeter; //!< The number of pixels per meter in the vertical direction
	uint32_t numColorsInTable; //!< The number of colors included in the provided table
	uint32_t numImportantColors; //!< The number of the colors in the table which are important
};

/// <summary>Structure of a info header2 for a BMP file</summary>
struct InfoHeader2 : public InfoHeader1 // Adobe Specific
{
	uint32_t redMask; //!< A red channel mask
	uint32_t greenMask; //!< A green channel mask
	uint32_t blueMask; //!< A blue channel mask
};

/// <summary>Structure of a info header3 for a BMP file</summary>
struct InfoHeader3 : public InfoHeader2 // Adobe Specific
{
	uint32_t alphaMask; //!< An alpha channel mask
};

/// <summary>Simple ivec3 representation</summary>
struct ivec3
{
	int32_t x; //!< x component
	int32_t y; //!< y component
	int32_t z; //!< z component
};

/// <summary>Structure of a info header4 for a BMP file</summary>
struct InfoHeader4 : public InfoHeader3
{
	int32_t colorSpace; //!< The color space being employed
	ivec3 xyzEndPoints[3]; //!< XYZ component end points
	uint32_t gammaRed; //!< Gamma used by the red component
	uint32_t gammaGreen; //!< Gamma used by the green component
	uint32_t gammaBlue; //!< Gamma used by the blue component
};

/// <summary>Structure of a info header5 for a BMP file</summary>
struct InfoHeader5 : public InfoHeader4
{
	uint32_t intent; //!< The intent
	uint32_t profileData; //!< Profile data
	uint32_t profileSize; //!< The profile size
	uint32_t reserved; //!< Reserved
};

namespace HeaderSize {
/// <summary>Enumeration for various BMP file specific header sizes and their offsets</summary>
enum Enum
{
	File = 14,
	Core = 12,
	Core2 = 64,
	Info1 = 40,
	Info2 = 52,
	Info3 = 56,
	Info4 = 108,
	Info5 = 124
};
} // namespace HeaderSize

namespace CompressionMethod {
/// <summary>Enumeration for various BMP file specific compression methods</summary>
enum Enum
{
	None,
	RunLength8,
	RunLength4,
	Bitfields,
	JPEG,
	PNG,
	AlphaBitfields
};
} // namespace CompressionMethod

namespace ColorSpace {
/// <summary>Enumeration for various BMP file specific color spaces</summary>
enum Enum
{
	CalibratedRGB = 0, // Gamma correction values are supplied.
	sRGB = 0x42475273, // 'sRGB' in ASCII
	Windows = 0x206e6957, // 'Win ' in ASCII
	ProfileLinked = 0x4b4e494c, // 'LINK' in ASCII
	ProfileEmbedded = 0x4445424d // 'MBED' in ASCII
};
} // namespace ColorSpace

/// <summary>Identifier used to identify BMP files</summary>
static const uint16_t Identifier = 0x4d42; // 'B' 'M' in ASCII
} // namespace texture_bmp
} // namespace pvr
