/*!
\brief Implementation of methods of the TextureHeader class.
\file PVRCore/texture/TextureHeader.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRCore/texture/TextureHeader.h"
#include "PVRCore/textureio/FileDefinesDDS.h"
#include "PVRCore/texture/PixelFormat.h"
#include "PVRCore/Log.h"
#include <algorithm>
using std::string;
using std::map;
namespace pvr {

TextureHeader::TextureHeader()
{
	flags = 0;
	pixelFormat = CompressedPixelFormat::NumCompressedPFs;
	colorSpace = ColorSpace::lRGB;
	channelType = VariableType::UnsignedByteNorm;
	height = 1;
	width = 1;
	depth = 1;
	numSurfaces = 1;
	numFaces = 1;
	numMipMaps = 1;
	metaDataSize = 0;
}

/*
TextureHeader::TextureHeader(Header fileHeader, uint32_t numMetaData, TextureMetaData* metaData) : _header(fileHeader)
{
	if (metaData)
	{
		for (uint32_t i = 0; i < numMetaData; ++i) { addMetaData(metaData[i]); }
	}
}
*/

TextureHeader::TextureHeader(PixelFormat pixelFormat, uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, ColorSpace colorSpace, VariableType channelType,
	uint32_t numSurfaces, uint32_t numFaces, uint32_t flags, TextureMetaData* metaData, uint32_t metaDataSize)
{
	this->pixelFormat = pixelFormat;
	this->width = width, this->height = height, this->depth = depth;
	this->numMipMaps = numMipMaps;
	this->colorSpace = colorSpace;
	this->channelType = channelType;
	this->numSurfaces = numSurfaces;
	this->numFaces = numFaces;
	this->flags = flags;
	if (metaData)
	{
		for (uint32_t i = 0; i < metaDataSize; ++i) { addMetaData(metaData[i]); }
	}
}

const std::string TextureHeader::getCubeMapOrder() const
{
	// Make sure the meta block exists
	map<uint32_t, map<uint32_t, TextureMetaData>>::const_iterator foundFourCC = _metaDataMap.find(PVRv3);
	if (getNumFaces() <= 1) { throw InvalidOperationError("TextureHeader::getCubeMapOrder: Request for cube map order on non-cubemap Texture"); }

	if (foundFourCC != _metaDataMap.end())
	{
		map<uint32_t, TextureMetaData>::const_iterator foundMetaData = (foundFourCC->second).find(TextureMetaData::IdentifierCubeMapOrder);
		if (foundMetaData != (foundFourCC->second).end())
		{
			char cubeMapOrder[7];
			cubeMapOrder[6] = 0;
			memcpy(cubeMapOrder, (foundMetaData->second).getData(), 6);
			return std::string(cubeMapOrder);
		}
	}

	std::string defaultOrder("XxYyZz");

	// Remove characters for faces that don't exist
	defaultOrder.resize(defaultOrder.size() - 6 - getNumFaces());

	return defaultOrder;
}

uint32_t TextureHeader::getBitsPerPixel() const
{
	if (getPixelFormat().getPart().High != 0)
	{ return getPixelFormat().getPixelTypeChar()[4] + getPixelFormat().getPixelTypeChar()[5] + getPixelFormat().getPixelTypeChar()[6] + getPixelFormat().getPixelTypeChar()[7]; }
	else
	{
		switch (getPixelFormat().getPixelTypeId())
		{
		case static_cast<uint64_t>(CompressedPixelFormat::BW1bpp): return 1;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp): return 2;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp):
		case static_cast<uint64_t>(CompressedPixelFormat::ETC1):
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_R11):
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB_A1):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT1):
		case static_cast<uint64_t>(CompressedPixelFormat::BC4): return 4;
		case static_cast<uint64_t>(CompressedPixelFormat::DXT2):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT3):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT4):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT5):
		case static_cast<uint64_t>(CompressedPixelFormat::BC5):
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_RG11):
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGBA): return 8;
		case static_cast<uint64_t>(CompressedPixelFormat::YUY2):
		case static_cast<uint64_t>(CompressedPixelFormat::UYVY):
		case static_cast<uint64_t>(CompressedPixelFormat::RGBG8888):
		case static_cast<uint64_t>(CompressedPixelFormat::GRGB8888): return 16;
		case static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5): return 32;
		default: return 0;
		}
	}
}

void TextureHeader::getMinDimensionsForFormat(uint32_t& minX, uint32_t& minY, uint32_t& minZ) const
{
	if (getPixelFormat().getPart().High != 0)
	{
		// Non-compressed formats all return 1.
		minX = minY = minZ = 1;
	}
	else
	{
		// Default
		minX = minY = minZ = 1;

		switch (getPixelFormat().getPixelTypeId())
		{
		case static_cast<uint64_t>(CompressedPixelFormat::DXT1):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT2):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT3):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT4):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT5):
		case static_cast<uint64_t>(CompressedPixelFormat::BC4):
		case static_cast<uint64_t>(CompressedPixelFormat::BC5):
		case static_cast<uint64_t>(CompressedPixelFormat::ETC1):
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGBA):
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB_A1):
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_R11):
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_RG11):
			minX = 4;
			minY = 4;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA):
			minX = 8;
			minY = 8;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA):
			minX = 16;
			minY = 8;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp):
			minX = 4;
			minY = 4;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp):
			minX = 8;
			minY = 4;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::UYVY):
		case static_cast<uint64_t>(CompressedPixelFormat::YUY2):
		case static_cast<uint64_t>(CompressedPixelFormat::RGBG8888):
		case static_cast<uint64_t>(CompressedPixelFormat::GRGB8888):
			minX = 2;
			minY = 1;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::BW1bpp):
			minX = 8;
			minY = 1;
			minZ = 1;
			break;
			// Error
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x4):
			minX = 4;
			minY = 4;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x4):
			minX = 5;
			minY = 4;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x5):
			minX = 5;
			minY = 5;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x5):
			minX = 6;
			minY = 5;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x6):
			minX = 6;
			minY = 6;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x5):
			minX = 8;
			minY = 5;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x6):
			minX = 8;
			minY = 6;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x8):
			minX = 8;
			minY = 8;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x5):
			minX = 10;
			minY = 5;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x6):
			minX = 10;
			minY = 6;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x8):
			minX = 10;
			minY = 8;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x10):
			minX = 10;
			minY = 10;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_12x10):
			minX = 12;
			minY = 10;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_12x12):
			minX = 12;
			minY = 12;
			minZ = 1;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_3x3x3):
			minX = 3;
			minY = 3;
			minZ = 3;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x3x3):
			minX = 4;
			minY = 3;
			minZ = 3;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x4x3):
			minX = 4;
			minY = 4;
			minZ = 3;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x4x4):
			minX = 4;
			minY = 4;
			minZ = 4;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x4x4):
			minX = 5;
			minY = 4;
			minZ = 4;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x5x4):
			minX = 5;
			minY = 5;
			minZ = 4;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x5x5):
			minX = 5;
			minY = 5;
			minZ = 5;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x5x5):
			minX = 6;
			minY = 5;
			minZ = 5;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x6x5):
			minX = 6;
			minY = 6;
			minZ = 5;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x6x6):
			minX = 6;
			minY = 6;
			minZ = 6;
			break;
		case static_cast<uint64_t>(CompressedPixelFormat::NumCompressedPFs): break;
		}
	}
}

void TextureHeader::setBumpMap(float bumpScale, std::string bumpOrder)
{
	if (bumpOrder.find_first_not_of("xyzh") != std::string::npos)
	{
		assertion(false, "Invalid Bumpmap order std::string");
		Log("Invalid Bumpmap order std::string");
		return;
	}
	// Get a reference to the meta data block.
	TextureMetaData& bumpMetaData = _metaDataMap[PVRv3][TextureMetaData::IdentifierBumpData];

	// Check if it's already been set or not.
	if (bumpMetaData.getData()) { metaDataSize -= bumpMetaData.getTotalSizeInMemory(); }

	// Initialize and clear the bump map data
	char bumpData[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	// Copy the floating point scale and character order into the bumpmap data
	memcpy(bumpData, &bumpScale, 4);
	memcpy(bumpData + 4, bumpOrder.c_str(), (std::min)(bumpOrder.length(), size_t(4)));

	bumpMetaData = TextureMetaData(PVRv3, TextureMetaData::IdentifierBumpData, 8, bumpData);

	// Increment the meta data size.
	metaDataSize += bumpMetaData.getTotalSizeInMemory();
}

TextureMetaData::AxisOrientation TextureHeader::getOrientation(TextureMetaData::Axis axis) const
{
	// Make sure the meta block exists

	std::map<uint32_t, std::map<uint32_t, TextureMetaData>>::const_iterator foundIdentifer = _metaDataMap.find(PVRv3);
	if (foundIdentifer != _metaDataMap.end())
	{
		std::map<uint32_t, TextureMetaData>::const_iterator foundTexMetaData = foundIdentifer->second.find(TextureMetaData::IdentifierTextureOrientation);
		if (foundTexMetaData != foundIdentifer->second.end()) { return static_cast<TextureMetaData::AxisOrientation>(foundTexMetaData->second.getData()[axis]); }
	}

	return static_cast<TextureMetaData::AxisOrientation>(0); // Default is the flag values.
}

bool TextureHeader::getDirectXGIFormat(uint32_t& dxgiFormat, bool& notAlpha) const
{
	// Default value in case of errors
	dxgiFormat = texture_dds::DXGI_FORMAT_UNKNOWN;
	notAlpha = false;
	if (getPixelFormat().getPart().High == 0)
	{
		if (getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::RGBG8888))
		{
			dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_B8G8_UNORM;
			return true;
		}
		if (getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::GRGB8888))
		{
			dxgiFormat = texture_dds::DXGI_FORMAT_G8R8_G8B8_UNORM;
			return true;
		}
		if (getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::BW1bpp))
		{
			dxgiFormat = texture_dds::DXGI_FORMAT_R1_UNORM;
			return true;
		}
		if (getChannelType() == VariableType::UnsignedIntegerNorm || getChannelType() == VariableType::UnsignedShortNorm || getChannelType() == VariableType::UnsignedByteNorm)
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case static_cast<uint64_t>(CompressedPixelFormat::BC1):
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC1_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC1_UNORM;
					return true;
				}
			case static_cast<uint64_t>(CompressedPixelFormat::BC2):
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC2_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC2_UNORM;
					return true;
				}
			case static_cast<uint64_t>(CompressedPixelFormat::BC3):
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC3_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC3_UNORM;
					return true;
				}
			case static_cast<uint64_t>(CompressedPixelFormat::BC4): dxgiFormat = texture_dds::DXGI_FORMAT_BC4_UNORM; return true;
			case static_cast<uint64_t>(CompressedPixelFormat::BC5): dxgiFormat = texture_dds::DXGI_FORMAT_BC5_UNORM; return true;
			}
		}
		else if (getChannelType() == VariableType::SignedIntegerNorm || getChannelType() == VariableType::SignedShortNorm || getChannelType() == VariableType::SignedByteNorm)
		{
			if (getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::BC4))
			{
				dxgiFormat = texture_dds::DXGI_FORMAT_BC4_SNORM;
				return true;
			}
			if (getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::BC5))
			{
				dxgiFormat = texture_dds::DXGI_FORMAT_BC5_SNORM;
				return true;
			}
		}
	}
	else
	{
		switch (getChannelType())
		{
		case VariableType::SignedFloat: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 32, 32, 32, 32>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32A32_FLOAT; return true;
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32_FLOAT; return true;
			case GeneratePixelType2<'r', 'g', 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32_FLOAT; return true;
			case GeneratePixelType1<'r', 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32_FLOAT; return true;
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_FLOAT; return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_FLOAT; return true;
			case GeneratePixelType1<'r', 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16_FLOAT; return true;
			case GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R11G11B10_FLOAT; return true;
			}
			break;
		}
		case VariableType::UnsignedByte: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_UINT; return true;
			case GeneratePixelType2<'r', 'g', 8, 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_UINT; return true;
			case GeneratePixelType1<'r', 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8_UINT; return true;
			}
			break;
		}
		case VariableType::UnsignedByteNorm: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: {
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_UNORM;
					return true;
				}
			}
			case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID: {
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_B8G8R8A8_UNORM;
					return true;
				}
			}
			case GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID: {
				notAlpha = true;
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_B8G8R8X8_UNORM;
					return true;
				}
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_UNORM; return true;
			case GeneratePixelType1<'r', 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8_UNORM; return true;
			case GeneratePixelType1<'x', 8>::ID: notAlpha = true;
			case GeneratePixelType1<'a', 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_A8_UNORM; return true;
			}
			break;
		}
		case VariableType::SignedByte: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_SINT; return true;
			case GeneratePixelType2<'r', 'g', 8, 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_SINT; return true;
			case GeneratePixelType1<'r', 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8_SINT; return true;
			}
			break;
		}
		case VariableType::SignedByteNorm: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_SNORM; return true;
			case GeneratePixelType2<'r', 'g', 8, 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_SNORM; return true;
			case GeneratePixelType1<'r', 8>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R8_SNORM; return true;
			}
			break;
		}
		case VariableType::UnsignedShort: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_UINT; return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_UINT; return true;
			case GeneratePixelType1<'r', 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16_UINT; return true;
			}
			break;
		}
		case VariableType::UnsignedShortNorm: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_UNORM; return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_UNORM; return true;
			case GeneratePixelType1<'r', 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16_UNORM; return true;
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_B5G6R5_UNORM; return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 5, 5, 5, 1>::ID: notAlpha = true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 5, 5, 5, 1>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_B5G5R5A1_UNORM; return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 4, 4, 4, 4>::ID: notAlpha = true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_B4G4R4A4_UNORM; return true;
			}
			break;
		}
		case VariableType::SignedShort: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_SINT; return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_SINT; return true;
			case GeneratePixelType1<'r', 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16_SINT; return true;
			}
			break;
		}
		case VariableType::SignedShortNorm: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_SNORM; return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_SNORM; return true;
			case GeneratePixelType1<'r', 16>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R16_SNORM; return true;
			}
			break;
		}
		case VariableType::UnsignedInteger: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 32, 32, 32, 32>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32A32_UINT; return true;
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32_UINT; return true;
			case GeneratePixelType2<'r', 'g', 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32_UINT; return true;
			case GeneratePixelType1<'r', 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32_UINT; return true;
			case GeneratePixelType4<'r', 'g', 'b', 'x', 10, 10, 10, 2>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R10G10B10A2_UINT; return true;
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm: {
			if (getPixelFormat().getPixelTypeId() == GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID)
			{
				dxgiFormat = texture_dds::DXGI_FORMAT_R10G10B10A2_UNORM;
				return true;
			}
			if (getPixelFormat().getPixelTypeId() == GeneratePixelType4<'r', 'g', 'b', 'x', 10, 10, 10, 2>::ID)
			{
				notAlpha = true;
				dxgiFormat = texture_dds::DXGI_FORMAT_R10G10B10A2_UNORM;
				return true;
			}
			break;
		}
		case VariableType::SignedInteger: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 32, 32, 32, 32>::ID: notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32A32_SINT; return true;
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32_SINT; return true;
			case GeneratePixelType2<'r', 'g', 32, 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32G32_SINT; return true;
			case GeneratePixelType1<'r', 32>::ID: dxgiFormat = texture_dds::DXGI_FORMAT_R32_SINT; return true;
			}
			break;
		}
		default: {
		}
		}
	}

	// Default return value if no errors.
	return false;
}

bool TextureHeader::getDirect3DFormat(uint32_t& d3dFormat) const
{
	// Default return value if unrecognised
	d3dFormat = texture_dds::D3DFMT_UNKNOWN;

	if (getPixelFormat().getPart().High == 0)
	{
		switch (getPixelFormat().getPixelTypeId())
		{
		case static_cast<uint64_t>(CompressedPixelFormat::DXT1): d3dFormat = texture_dds::D3DFMT_DXT1; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::DXT2): d3dFormat = texture_dds::D3DFMT_DXT2; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::DXT3): d3dFormat = texture_dds::D3DFMT_DXT3; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::DXT4): d3dFormat = texture_dds::D3DFMT_DXT4; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::DXT5): d3dFormat = texture_dds::D3DFMT_DXT5; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA): d3dFormat = texture_dds::D3DFMT_PVRTC2; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA): d3dFormat = texture_dds::D3DFMT_PVRTC4; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::YUY2): d3dFormat = texture_dds::D3DFMT_YUY2; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::UYVY): d3dFormat = texture_dds::D3DFMT_UYVY; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::RGBG8888): d3dFormat = texture_dds::D3DFMT_R8G8_B8G8; return true;
		case static_cast<uint64_t>(CompressedPixelFormat::GRGB8888): d3dFormat = texture_dds::D3DFMT_G8R8_G8B8; return true;
		}
	}
	else
	{
		switch (getChannelType())
		{
		case VariableType::SignedFloat: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType1<'r', 16>::ID: d3dFormat = texture_dds::D3DFMT_R16F; return true;
			case GeneratePixelType2<'g', 'r', 16, 16>::ID: d3dFormat = texture_dds::D3DFMT_G16R16F; return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID: d3dFormat = texture_dds::D3DFMT_A16B16G16R16F; return true;
			case GeneratePixelType1<'r', 32>::ID: d3dFormat = texture_dds::D3DFMT_R32F; return true;
			case GeneratePixelType2<'r', 'g', 32, 32>::ID: d3dFormat = texture_dds::D3DFMT_G32R32F; return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 32, 32, 32, 32>::ID: d3dFormat = texture_dds::D3DFMT_A32B32G32R32F; return true;
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_R8G8B8; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_A8R8G8B8; return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 8, 8, 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_X8R8G8B8; return true;
			case GeneratePixelType2<'a', 'l', 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_A8L8; return true;
			case GeneratePixelType1<'a', 8>::ID: d3dFormat = texture_dds::D3DFMT_A8; return true;
			case GeneratePixelType1<'l', 8>::ID: d3dFormat = texture_dds::D3DFMT_L8; return true;
			case GeneratePixelType2<'a', 'l', 4, 4>::ID: d3dFormat = texture_dds::D3DFMT_A4L4; return true;
			case GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID: d3dFormat = texture_dds::D3DFMT_R3G3B2; return true;
			case GeneratePixelType1<'l', 16>::ID: d3dFormat = texture_dds::D3DFMT_L16; return true;
			case GeneratePixelType2<'g', 'r', 16, 16>::ID: d3dFormat = texture_dds::D3DFMT_G16R16; return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID: d3dFormat = texture_dds::D3DFMT_A16B16G16R16; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID: d3dFormat = texture_dds::D3DFMT_A4R4G4B4; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID: d3dFormat = texture_dds::D3DFMT_A1R5G5B5; return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 1, 5, 5, 5>::ID: d3dFormat = texture_dds::D3DFMT_X1R5G5B5; return true;
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID: d3dFormat = texture_dds::D3DFMT_R5G6B5; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 3, 3, 2>::ID: d3dFormat = texture_dds::D3DFMT_A8R3G3B2; return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID: d3dFormat = texture_dds::D3DFMT_A2B10G10R10; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 2, 10, 10, 10>::ID: d3dFormat = texture_dds::D3DFMT_A2R10G10B10; return true;
			}
			break;
		}
		case VariableType::UnsignedByteNorm: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_R8G8B8; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_A8R8G8B8; return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 8, 8, 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_X8R8G8B8; return true;
			case GeneratePixelType2<'a', 'l', 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_A8L8; return true;
			case GeneratePixelType1<'a', 8>::ID: d3dFormat = texture_dds::D3DFMT_A8; return true;
			case GeneratePixelType1<'l', 8>::ID: d3dFormat = texture_dds::D3DFMT_L8; return true;
			case GeneratePixelType2<'a', 'l', 4, 4>::ID: d3dFormat = texture_dds::D3DFMT_A4L4; return true;
			case GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID: d3dFormat = texture_dds::D3DFMT_R3G3B2; return true;
			}
			break;
		}
		case VariableType::UnsignedShortNorm: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType1<'l', 16>::ID: d3dFormat = texture_dds::D3DFMT_L16; return true;
			case GeneratePixelType2<'g', 'r', 16, 16>::ID: d3dFormat = texture_dds::D3DFMT_G16R16; return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID: d3dFormat = texture_dds::D3DFMT_A16B16G16R16; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID: d3dFormat = texture_dds::D3DFMT_A4R4G4B4; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID: d3dFormat = texture_dds::D3DFMT_A1R5G5B5; return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 1, 5, 5, 5>::ID: d3dFormat = texture_dds::D3DFMT_X1R5G5B5; return true;
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID: d3dFormat = texture_dds::D3DFMT_R5G6B5; return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 3, 3, 2>::ID: d3dFormat = texture_dds::D3DFMT_A8R3G3B2; return true;
			}
			break;
		}
		case VariableType::SignedIntegerNorm: {
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType2<'g', 'r', 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_V8U8; return true;
			case GeneratePixelType4<'x', 'l', 'g', 'r', 8, 8, 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_X8L8V8U8; return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID: d3dFormat = texture_dds::D3DFMT_Q8W8V8U8; return true;
			case GeneratePixelType3<'l', 'g', 'r', 6, 5, 5>::ID: d3dFormat = texture_dds::D3DFMT_L6V5U5; return true;
			case GeneratePixelType2<'g', 'r', 16, 16>::ID: d3dFormat = texture_dds::D3DFMT_V16U16; return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID: d3dFormat = texture_dds::D3DFMT_A2W10V10U10; return true;
			}
			break;
		}
		default: break;
		}
	}

	// Return false if format recognised
	return false;
}

uint32_t TextureHeader::getDataSize(int32_t iMipLevel, bool bAllSurfaces, bool bAllFaces) const
{
	// The smallest divisible sizes for a pixel format
	uint32_t uiSmallestWidth = 1;
	uint32_t uiSmallestHeight = 1;
	uint32_t uiSmallestDepth = 1;

	// Get the pixel format's minimum dimensions.
	getMinDimensionsForFormat(uiSmallestWidth, uiSmallestHeight, uiSmallestDepth);

	// Needs to be 64-bit Integer to support 16kx16k and higher sizes.
	uint64_t uiDataSize = 0;
	if (iMipLevel == -1)
	{
		for (uint32_t uiCurrentMipMap = 0; uiCurrentMipMap < getNumMipMapLevels(); ++uiCurrentMipMap)
		{
			// Get the dimensions of the current MIP Map level.
			uint32_t uiWidth = getWidth(uiCurrentMipMap);
			uint32_t uiHeight = getHeight(uiCurrentMipMap);
			uint32_t uiDepth = getDepth(uiCurrentMipMap);
			uint32_t bpp = getBitsPerPixel();

			// If pixel format is compressed, the dimensions need to be padded.
			if (getPixelFormat().getPart().High == 0)
			{
				uiWidth = uiWidth + ((-1 * uiWidth) % uiSmallestWidth);
				uiHeight = uiHeight + ((-1 * uiHeight) % uiSmallestHeight);
				uiDepth = uiDepth + ((-1 * uiDepth) % uiSmallestDepth);
			}

			// Add the current MIP Map's data size to the total.
			if (getPixelFormat().getPixelTypeId() >= (uint64_t)CompressedPixelFormat::ASTC_4x4 && getPixelFormat().getPixelTypeId() <= (uint64_t)CompressedPixelFormat::ASTC_6x6x6)
			{ uiDataSize += (uiWidth / uiSmallestWidth) * (uiHeight / uiSmallestHeight) * (uiDepth / uiSmallestDepth) * 128; }
			else
			{
				uiDataSize += bpp * static_cast<uint64_t>(uiWidth) * static_cast<uint64_t>(uiHeight) * static_cast<uint64_t>(uiDepth);
			}
		}
	}
	else
	{
		// Get the dimensions of the specified MIP Map level.
		uint32_t uiWidth = getWidth(iMipLevel);
		uint32_t uiHeight = getHeight(iMipLevel);
		uint32_t uiDepth = getDepth(iMipLevel);

		// If pixel format is compressed, the dimensions need to be padded.
		if (getPixelFormat().getPart().High == 0)
		{
			uiWidth = uiWidth + ((-1 * uiWidth) % uiSmallestWidth);
			uiHeight = uiHeight + ((-1 * uiHeight) % uiSmallestHeight);
			uiDepth = uiDepth + ((-1 * uiDepth) % uiSmallestDepth);
		}

		// Work out the specified MIP Map's data size
		uiDataSize = static_cast<uint64_t>(getBitsPerPixel()) * static_cast<uint64_t>(uiWidth) * static_cast<uint64_t>(uiHeight) * static_cast<uint64_t>(uiDepth);
	}

	// The number of faces/surfaces to register the size of.
	uint32_t numfaces = (bAllFaces ? getNumFaces() : 1);
	uint32_t numsurfs = (bAllSurfaces ? getNumArrayMembers() : 1);

	// Multiply the data size by number of faces and surfaces specified, and return.
	return static_cast<uint32_t>(uiDataSize / 8) * numsurfs * numfaces;
}

ptrdiff_t TextureHeader::getDataOffset(uint32_t mipMapLevel /*= 0*/, uint32_t arrayMember /*= 0*/, uint32_t face /*= 0*/) const
{
	// Initialize the offSet value.
	uint32_t uiOffSet = 0;

	// Error checking
	if ((static_cast<int32_t>(mipMapLevel) == pvrTextureAllMipMaps) || mipMapLevel >= getNumMipMapLevels())
	{ throw InvalidArgumentError("mipmapLevel", "TextureHeader::getDataOffset: Specified mipmap level did not exist"); }
	if (arrayMember >= getNumArrayMembers()) { throw InvalidArgumentError("arrayMember", "TextureHeader::getDataOffset: Specified array member did not exist"); }
	if (face >= getNumFaces())
	{ throw InvalidArgumentError("face", "TextureHeader::getDataOffset: Specified face did not exist"); } // File is organised by MIP Map levels, then surfaces, then faces.

	// Get the start of the MIP level.
	if (mipMapLevel != 0)
	{
		// Get the size for all MIP Map levels up to this one.
		for (uint32_t uiCurrentMipMap = 0; uiCurrentMipMap < mipMapLevel; ++uiCurrentMipMap) { uiOffSet += getDataSize(uiCurrentMipMap, true, true); }
	}

	// Get the start of the array.
	if (arrayMember != 0) { uiOffSet += arrayMember * getDataSize(mipMapLevel, false, true); }

	// Get the start of the face.
	if (face != 0) { uiOffSet += face * getDataSize(mipMapLevel, false, false); }

	// Return the data pointer plus whatever offSet has been specified.
	return uiOffSet;
}

void TextureHeader::setOrientation(TextureMetaData::AxisOrientation eAxisOrientation)
{
	// Get a reference to the meta data block.
	TextureMetaData& orientationMetaData = _metaDataMap[PVRv3][TextureMetaData::IdentifierTextureOrientation];

	// Check if it's already been set or not.
	if (orientationMetaData.getData()) { metaDataSize -= orientationMetaData.getTotalSizeInMemory(); }

	// Set the orientation data
	char orientationData[3];

	// Check for left/right (x-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationLeft) > 0) { orientationData[TextureMetaData::AxisAxisX] = TextureMetaData::AxisOrientationLeft; }
	else
	{
		orientationData[TextureMetaData::AxisAxisX] = TextureMetaData::AxisOrientationRight;
	}

	// Check for up/down (y-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationUp) > 0) { orientationData[TextureMetaData::AxisAxisY] = TextureMetaData::AxisOrientationUp; }
	else
	{
		orientationData[TextureMetaData::AxisAxisY] = TextureMetaData::AxisOrientationDown;
	}

	// Check for in/out (z-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationOut) > 0) { orientationData[TextureMetaData::AxisAxisZ] = TextureMetaData::AxisOrientationOut; }
	else
	{
		orientationData[TextureMetaData::AxisAxisZ] = TextureMetaData::AxisOrientationIn;
	}

	// Update the meta data block
	orientationMetaData = TextureMetaData(PVRv3, TextureMetaData::IdentifierTextureOrientation, 3, orientationData);

	// Check that the meta data was created successfully.
	if (orientationMetaData.getDataSize() != 0)
	{
		// Increment the meta data size.
		metaDataSize += orientationMetaData.getTotalSizeInMemory();
	}
	else
	{
		// Otherwise remove it.
		_metaDataMap.erase(TextureMetaData::IdentifierTextureOrientation);
	}
}

void TextureHeader::setCubeMapOrder(std::string cubeMapOrder)
{
	if (cubeMapOrder.find_first_not_of("xXyYzZ") != std::string::npos)
	{ throw InvalidArgumentError("cubeMapOrder", "TextureHeader::setCubeMapOrder: Specified cubemap order string was invalid."); } // Get a reference to the meta data block.
	TextureMetaData& cubeOrderMetaData = _metaDataMap[PVRv3][TextureMetaData::IdentifierCubeMapOrder];

	// Check if it's already been set or not.
	if (cubeOrderMetaData.getData()) { metaDataSize -= cubeOrderMetaData.getTotalSizeInMemory(); }

	cubeOrderMetaData =
		TextureMetaData(PVRv3, TextureMetaData::IdentifierCubeMapOrder, (std::min)(static_cast<uint32_t>(cubeMapOrder.length()), 6u), static_cast<const char*>(cubeMapOrder.data()));

	// Increment the meta data size.
	metaDataSize += cubeOrderMetaData.getTotalSizeInMemory();
}

void TextureHeader::addMetaData(const TextureMetaData& metaData)
{
	// Get a reference to the meta data block.
	TextureMetaData& currentMetaData = _metaDataMap[metaData.getFourCC()][metaData.getKey()];

	// Check if it's already been set or not.
	if (currentMetaData.getData()) { metaDataSize -= currentMetaData.getTotalSizeInMemory(); }

	// Set the meta data block
	currentMetaData = metaData;

	// Increment the meta data size.
	metaDataSize += currentMetaData.getTotalSizeInMemory();
}

bool TextureHeader::isBumpMap() const
{
	const std::map<uint32_t, TextureMetaData>& dataMap = _metaDataMap.at(PVRv3);
	return (dataMap.find(TextureMetaData::IdentifierBumpData) != dataMap.end());
}

} // namespace pvr
//!\endcond
