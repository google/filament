/*!
\brief Implementation of methods of the TextureReaderTGA class.
\file PVRCore/textureio/TextureReaderTGA.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/textureio/TextureReaderTGA.h"
#include "PVRCore/textureio/PaletteExpander.h"
#include <algorithm>
using std::vector;
namespace pvr {
namespace assetReaders {
namespace {
texture_tga::FileHeader readFileHeader(const pvr::Stream& stream)
{
	texture_tga::FileHeader fileheader;
	// Read the size of the identifier area
	stream.readExact(sizeof(fileheader.identSize), 1, &fileheader.identSize);
	// Read the color map type
	stream.readExact(sizeof(fileheader.colorMapType), 1, &fileheader.colorMapType);
	// Read the image type
	stream.readExact(sizeof(fileheader.imageType), 1, &fileheader.imageType);
	// Read the start position of the color map
	stream.readExact(sizeof(fileheader.colorMapStart), 1, &fileheader.colorMapStart);
	// Read the length of the color map
	stream.readExact(sizeof(fileheader.colorMapLength), 1, &fileheader.colorMapLength);
	// Read the number of bits per palette entry in the color map
	stream.readExact(sizeof(fileheader.colorMapBits), 1, &fileheader.colorMapBits);
	// Read the horizontal offset for the start of the image
	stream.readExact(sizeof(fileheader.xStart), 1, &fileheader.xStart);
	// Read the vertical offset for the start of the image
	stream.readExact(sizeof(fileheader.yStart), 1, &fileheader.yStart);
	// Read the width of the image
	stream.readExact(sizeof(fileheader.width), 1, &fileheader.width);
	// Read the height of the image
	stream.readExact(sizeof(fileheader.height), 1, &fileheader.height);
	// Read the bits per pixel in the image
	stream.readExact(sizeof(fileheader.bits), 1, &fileheader.bits);
	// Read the descriptor flags
	stream.readExact(sizeof(fileheader.descriptor), 1, &fileheader.descriptor);
	return fileheader;
}

void loadIndexed(const texture_tga::FileHeader& header, const Stream& stream, Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bytesPerDataEntry)
{
	// Check that a palette is present.
	if (header.colorMapType != texture_tga::ColorMap::Paletted)
	{
		throw InvalidOperationError("[TextureReaderTGA::loadIndexed]: Reading from [" + stream.getFileName() + "] - Image Type specifies palette data, but no palette is supplied.");
	}

	// Work out the size of the palette data entries
	uint32_t paletteEntries = static_cast<uint32_t>(header.colorMapLength - header.colorMapStart);
	uint32_t paletteSize = paletteEntries * bytesPerPaletteEntry;

	// Allocate data to read the palette into.
	std::vector<unsigned char> paletteData;
	paletteData.resize(paletteSize);

	// seek to the beginning of the palette
	stream.seek(static_cast<long>(header.colorMapStart * bytesPerPaletteEntry), Stream::SeekOriginFromCurrent);

	// Read the palette
	stream.readExact(bytesPerPaletteEntry, paletteEntries, paletteData.data());

	// Create the palette helper class
	PaletteExpander paletteLookup(paletteData.data(), paletteSize, bytesPerPaletteEntry);

	// Start reading data
	unsigned char* outputPixel = asset.getDataPointer();
	uint32_t currentIndex = 0;
	for (uint32_t texturePosition = 0; texturePosition < (asset.getTextureSize()); ++texturePosition)
	{
		// Read the index
		stream.readExact(bytesPerDataEntry, 1, &currentIndex);
		// Get the color output
		paletteLookup.getColorFromIndex(currentIndex, outputPixel);

		// Increment the pixel
		outputPixel += bytesPerPaletteEntry;
	}
}

void loadRunLength(const texture_tga::FileHeader& header, const Stream& stream, Texture& asset, uint32_t bytesPerDataEntry)
{
	(void)header;
	// Buffer for any repeated values come across
	vector<char> repeatedValue;
	repeatedValue.resize(bytesPerDataEntry);

	// Read the run length encoded data, and decode it.
	unsigned char* outputPixel = asset.getDataPointer();
	while (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
	{
		// Read the leading character for this block
		int8_t leadingCharacter;
		stream.readExact(1, 1, &leadingCharacter);
		// Check if it's a run of differing values or a run of the same value multiple times
		if (leadingCharacter >= 0)
		{
			// Read each value in turn
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Read in the value
					stream.readExact(bytesPerDataEntry, 1, outputPixel);
					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		else if (leadingCharacter > -128)
		{
			// Read a repeated value
			stream.readExact(bytesPerDataEntry, 1, repeatedValue.data());
			// Write the repeated value the appropriate number of times
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Copy the data across
					memcpy(outputPixel, repeatedValue.data(), bytesPerDataEntry);

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		// Character -128 is a "no op", so there's nothing to do for it. It's used as padding basically.
	}
}

void loadRunLengthIndexed(const texture_tga::FileHeader& header, const Stream& stream, Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bytesPerDataEntry)
{
	// Check that a palette is present.
	if (header.colorMapType != texture_tga::ColorMap::Paletted)
	{ throw InvalidDataError("[" + stream.getFileName() + "]: Image Type specifies palette data, but no palette is supplied."); }

	// Work out the size of the palette data entries
	uint32_t paletteEntries = (header.colorMapLength - header.colorMapStart);
	uint32_t paletteSize = paletteEntries * bytesPerPaletteEntry;

	// Allocate data to read the palette into.
	vector<uint8_t> paletteData;
	paletteData.resize(paletteSize);

	// seek to the beginning of the palette
	stream.seek(header.colorMapStart, Stream::SeekOriginFromCurrent);
	// Read the palette
	stream.readExact(bytesPerPaletteEntry, paletteEntries, paletteData.data());

	// Create the palette helper class
	PaletteExpander paletteLookup(paletteData.data(), paletteSize, bytesPerPaletteEntry);

	// Index value into the palette
	uint32_t currentIndex = 0;

	// Read the run length encoded data, and decode it.
	uint8_t* outputPixel = asset.getDataPointer();
	while (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
	{
		// Read the leading character for this block
		int8_t leadingCharacter;
		stream.readExact(1, 1, &leadingCharacter);
		// Check if it's a run of differing values or a run of the same value multiple times
		if (leadingCharacter >= 0)
		{
			// Read each index value in turn
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Read in the value
					stream.readExact(bytesPerDataEntry, 1, &currentIndex);
					// Get the color output
					paletteLookup.getColorFromIndex(currentIndex, outputPixel);

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		else if (leadingCharacter > -128)
		{
			// Read in the repeated value
			stream.readExact(bytesPerDataEntry, 1, &currentIndex);
			// Write the repeated value the appropriate number of times
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Get the color output
					paletteLookup.getColorFromIndex(currentIndex, outputPixel);

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		// Character -128 is a "no op", so there's nothing to do for it. It's used as padding basically.
	}
}

Texture loadImageFromFile(const Stream& stream, const texture_tga::FileHeader& header)
{
	Texture asset;
	// Setup the texture header.
	TextureHeader textureHeader;

	// Set the width and height from the file header.
	textureHeader.setWidth(header.width);
	textureHeader.setHeight(header.height);

	// Check whether the alpha value is ignored or not.
	bool alphaIgnored = ((header.descriptor & texture_tga::DescriptorFlagAlpha) == 0);

	// Get the bytes per data entry
	uint32_t bytesPerDataEntry = header.bits / 8;
	if (header.bits == 15) { bytesPerDataEntry = 2; }

	// Get the bytes per color map entry
	uint32_t bytesPerPaletteEntry = header.colorMapBits / 8;
	if (header.colorMapBits == 15) { bytesPerPaletteEntry = 2; }

	// Work out the bits per pixel of the final pixel format
	uint32_t bitsPerPixel = header.bits;
	if (header.colorMapType == texture_tga::ColorMap::Paletted) { bitsPerPixel = header.colorMapBits; }

	// Work out the pixel format - based on the number of bits in the final pixel format
	switch (bitsPerPixel)
	{
	case 8:
	{
		textureHeader.setPixelFormat(GeneratePixelType1<'l', 8>::ID);
		break;
	}
	case 15:
	{
		textureHeader.setPixelFormat(GeneratePixelType4<'x', 'b', 'g', 'r', 1, 5, 5, 5>::ID);
		textureHeader.setChannelType(VariableType::UnsignedShortNorm);
		break;
	}
	case 16:
	{
		if (alphaIgnored) { textureHeader.setPixelFormat(GeneratePixelType4<'x', 'b', 'g', 'r', 1, 5, 5, 5>::ID); }
		else
		{
			textureHeader.setPixelFormat(GeneratePixelType4<'a', 'b', 'g', 'r', 1, 5, 5, 5>::ID);
		}
		textureHeader.setChannelType(VariableType::UnsignedShortNorm);
		break;
	}
	case 24:
	{
		textureHeader.setPixelFormat(GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID);
		break;
	}
	case 32:
	{
		if (alphaIgnored) { textureHeader.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID); }
		else
		{
			textureHeader.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID);
		}
		break;
	}
	default:
		// Invalid format
		throw InvalidOperationError("[TextureReaderTGA::loadImageFromFile]: Reading from [" + stream.getFileName() + "] - Invalid number of bits per pixel in TGA file");
	}

	// Create the texture data
	asset = Texture(textureHeader);

	// Read the texture data according to how it's stored
	switch (header.imageType)
	{
	case texture_tga::ImageType::None:
	{
		memset(asset.getDataPointer(), 0, asset.getDataSize());
		break;
	}
	case texture_tga::ImageType::Indexed:
	{
		loadIndexed(header, stream, asset, bytesPerPaletteEntry, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RGB:
	case texture_tga::ImageType::GreyScale:
	{
		stream.readExact(bytesPerDataEntry, asset.getTextureSize(), asset.getDataPointer());
		break;
	}
	case texture_tga::ImageType::RunLengthIndexed:
	{
		loadRunLengthIndexed(header, stream, asset, bytesPerPaletteEntry, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RunLengthRGB:
	case texture_tga::ImageType::RunLengthGreyScale:
	{
		loadRunLength(header, stream, asset, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RunLengthHuffmanDelta:
	case texture_tga::ImageType::RunLengthHuffmanDeltaFourPassQuadTree:
	default:
	{
		throw InvalidOperationError("[TextureReaderTGA::loadImageFromFile]: Invalid image type");
		break;
	}
	}

	// Signify that the image has been loaded.
	return asset;
}
} // namespace

Texture readTGA(const Stream& stream)
{
	if (!stream.isReadable()) { throw InvalidOperationError("[pvr::assetReaders::readTGA] Attempted to read a non-readable assetStream"); }

	size_t original_position = stream.getPosition();
	try
	{
		texture_tga::FileHeader header(readFileHeader(stream));

		// Skip the identifier area
		stream.seek(header.identSize, Stream::SeekOriginFromCurrent);

		return loadImageFromFile(stream, header);
	}
	catch (...)
	{
		try
		{
			stream.seek((long)original_position, Stream::SeekOriginFromStart);
		}
		catch (...)
		{}
		throw;
	}
}

} // namespace assetReaders
} // namespace pvr
//!\endcond
