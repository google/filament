/*!
\brief Implementation of methods of the TextureReaderBMP class.
\file PVRCore/textureio/TextureReaderBMP.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/Log.h"
#include "PVRCore/textureio/TextureReaderBMP.h"
#include "PVRCore/textureio/PaletteExpander.h"
#include "PVRCore/texture/MetaData.h"
#include "PVRCore/textureio/FileDefinesBMP.h"

using std::string;
using std::vector;
namespace pvr {
namespace assetReaders {
namespace {

texture_bmp::FileHeader readFileHeader(const Stream& stream)
{
	texture_bmp::FileHeader fileheader;
	// Read the magic identifier
	stream.readExact(sizeof(fileheader.signature), 1, &fileheader.signature);
	// Make sure the signature is correct
	if (fileheader.signature != texture_bmp::Identifier) { throw InvalidArgumentError("TextureReaderBMP: Stream was not a valid BMP file"); }

	// Read the recorded file size
	stream.readExact(sizeof(fileheader.fileSize), 1, &fileheader.fileSize);
	// Read the first reserved data
	stream.readExact(sizeof(fileheader.reserved1), 1, &fileheader.reserved1);
	// Read the second reserved data
	stream.readExact(sizeof(fileheader.reserved2), 1, &fileheader.reserved2);
	// Read the pixel data offset from the beginning of the file
	stream.readExact(sizeof(fileheader.pixelOffset), 1, &fileheader.pixelOffset);
	return fileheader;
}
void loadRowAligned(const Stream& stream, Texture& asset, uint32_t bytesPerDataEntry, uint32_t rowAlignment)
{
	// Calculate the number of bytes to skip in each row
	uint32_t bytesPerScanline = asset.getWidth() * bytesPerDataEntry;
	uint32_t scanlinePadding = ((-1 * bytesPerScanline) % rowAlignment);

	// Start reading data
	unsigned char* outputPixel = asset.getDataPointer() + bytesPerScanline * (asset.getHeight() - 1);

	for (uint32_t y = 0; y < (asset.getHeight()); ++y)
	{
		// Read the next scan line
		stream.readExact(bytesPerDataEntry, asset.getWidth(), outputPixel);
		// Increment the pixel
		outputPixel -= bytesPerScanline;

		// seek past the scan line padding
		stream.seek(static_cast<long>(scanlinePadding), Stream::SeekOriginFromCurrent);
	}
}

void loadIndexed(const Stream& stream, const texture_bmp::FileHeader& fileheader, Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bitsPerDataEntry,
	uint32_t numPaletteEntries, uint32_t rowAlignment)
{
	// Work out the size of the palette data entries
	uint32_t paletteSize = numPaletteEntries * bytesPerPaletteEntry;

	// Allocate data to read the palette into.
	vector<unsigned char> paletteData;
	paletteData.resize(paletteSize);

	// Read the palette
	stream.readExact(bytesPerPaletteEntry, numPaletteEntries, &paletteData[0]);

	// Create the palette helper class
	PaletteExpander paletteLookup(&paletteData[0], paletteSize, bytesPerPaletteEntry);

	// seek to the pixel data
	stream.seek(fileheader.pixelOffset, Stream::SeekOriginFromStart);

	// Make sure that there are a POT number of bits.
	if (!((bitsPerDataEntry != 0) && (!(bitsPerDataEntry & (bitsPerDataEntry - 1)))))
	{ throw InvalidArgumentError("Reading from [" + stream.getFileName() + "] - Non-Power of two number of bits specified, unable to load."); }

	// Work out the number of indices per char, and bytes per index
	uint32_t indicesPerByte = 8 / bitsPerDataEntry;

	// Calculate the number of bytes to skip in each row
	uint32_t bytesPerScanline = (asset.getWidth() + (indicesPerByte - 1)) / indicesPerByte;
	uint32_t scanlinePadding = (((-1 * bytesPerScanline) % rowAlignment) * 8) / 8;

	// Work out the bit mask of the index value
	uint8_t indexMask = 0xffu >> (8 - bitsPerDataEntry);

	// Start reading data
	unsigned char* outputPixel = asset.getDataPointer();
	uint8_t currentIndexData = 0;
	for (uint32_t y = 0; y < (asset.getHeight()); ++y)
	{
		for (uint32_t x = 0; x < (asset.getWidth()); x += indicesPerByte)
		{
			// Read the next char of indices
			stream.readExact(1, 1, &currentIndexData);

			// Loop through all the indices in the char
			for (uint32_t indexPosition = 0; indexPosition < indicesPerByte; ++indexPosition)
			{
				if ((x + indexPosition) < asset.getWidth())
				{
					// Mask out the actual index from the current index data
					uint8_t bitShift = static_cast<uint8_t>(8 - (bitsPerDataEntry * (indexPosition + 1)));

					uint8_t actualIndex = static_cast<uint8_t>((currentIndexData & (indexMask << (bitShift))) >> (bitShift));

					// Get the color output - don't need to check for errors, as the out of bounds condition is impossible due to the mask.
					paletteLookup.getColorFromIndex(actualIndex, outputPixel);

					// Increment the pixel
					outputPixel += bytesPerPaletteEntry;
				}
			}
		}

		// seek past the scan line padding
		stream.seek(static_cast<long>(scanlinePadding), Stream::SeekOriginFromCurrent);
	}
}

void readCoreHeader(const Stream& stream, uint32_t headerSize, texture_bmp::CoreHeader& coreHeader)
{
	// Set the header size
	coreHeader.headerSize = headerSize;

	// Read the width
	stream.readExact(sizeof(coreHeader.width), 1, &coreHeader.width);
	// Read the height
	stream.readExact(sizeof(coreHeader.height), 1, &coreHeader.height);
	// Read the number of planes
	stream.readExact(sizeof(coreHeader.numPlanes), 1, &coreHeader.numPlanes);
	// Make sure the number of planes is one
	if (coreHeader.numPlanes != 1) { throw FileIOError(stream, "TextureReaderBMP::readCoreHeader: Number of planes was wrong"); }
	// Read the bits per pixel
	stream.readExact(sizeof(coreHeader.bitsPerPixel), 1, &coreHeader.bitsPerPixel);
}

void readInfoHeader(const Stream& stream, uint32_t headerSize, texture_bmp::InfoHeader5& infoHeader)
{
	// Set the header size
	infoHeader.headerSize = headerSize;

	// Read the width
	stream.readExact(sizeof(infoHeader.width), 1, &infoHeader.width);
	// Read the height
	stream.readExact(sizeof(infoHeader.height), 1, &infoHeader.height);
	// Read the number of planes
	stream.readExact(sizeof(infoHeader.numPlanes), 1, &infoHeader.numPlanes);
	// Make sure the number of planes is one
	if (infoHeader.numPlanes != 1) { throw FileIOError(stream, "TextureReaderBMP::readInfoHeader - Number of planes was invalid"); }
	// Read the bits per pixel
	stream.readExact(sizeof(infoHeader.bitsPerPixel), 1, &infoHeader.bitsPerPixel);
	// Read the compression type
	stream.readExact(sizeof(infoHeader.compressionType), 1, &infoHeader.compressionType);
	// Read the image size
	stream.readExact(sizeof(infoHeader.imageSize), 1, &infoHeader.imageSize);
	// Read the horizontal pixels per meter
	stream.readExact(sizeof(infoHeader.horizontalPixelsPerMeter), 1, &infoHeader.horizontalPixelsPerMeter);
	// Read the vertical pixels per meter
	stream.readExact(sizeof(infoHeader.verticalPixelsPerMeter), 1, &infoHeader.verticalPixelsPerMeter);
	// Read the number of colors in the table
	stream.readExact(sizeof(infoHeader.numColorsInTable), 1, &infoHeader.numColorsInTable);
	// Read the number of important colors in the table
	stream.readExact(sizeof(infoHeader.numImportantColors), 1, &infoHeader.numImportantColors);

	if (headerSize > texture_bmp::HeaderSize::Info1)
	{
		// Read the red mask
		stream.readExact(sizeof(infoHeader.redMask), 1, &infoHeader.redMask);
		// Read the green mask
		stream.readExact(sizeof(infoHeader.greenMask), 1, &infoHeader.greenMask);
		// Read the blue mask
		stream.readExact(sizeof(infoHeader.blueMask), 1, &infoHeader.blueMask);
		if (headerSize >= texture_bmp::HeaderSize::Info3)
		{
			// Read the alpha mask
			stream.readExact(sizeof(infoHeader.alphaMask), 1, &infoHeader.alphaMask);
		}
		if (headerSize >= texture_bmp::HeaderSize::Info4)
		{
			// Read the color space
			stream.readExact(sizeof(infoHeader.alphaMask), 1, &infoHeader.alphaMask);
			// Read the XYZ endpoints
			stream.readExact(sizeof(infoHeader.xyzEndPoints[0]), 3, infoHeader.xyzEndPoints);
			// Read the red gamma correction
			stream.readExact(sizeof(infoHeader.gammaRed), 1, &infoHeader.gammaRed);
			// Read the green gamma correction
			stream.readExact(sizeof(infoHeader.gammaGreen), 1, &infoHeader.gammaRed);
			// Read the blue gamma correction
			stream.readExact(sizeof(infoHeader.gammaBlue), 1, &infoHeader.gammaRed);
		}

		if (headerSize >= texture_bmp::HeaderSize::Info5)
		{
			// Read the intent
			stream.readExact(sizeof(infoHeader.intent), 1, &infoHeader.intent);
			// Read the profile data offset
			stream.readExact(sizeof(infoHeader.profileData), 1, &infoHeader.profileData);
			// Read the profile size
			stream.readExact(sizeof(infoHeader.profileSize), 1, &infoHeader.profileSize);
			// Read the reserved bit
			stream.readExact(sizeof(infoHeader.reserved), 1, &infoHeader.reserved);
		}
	}
}

void translateInfoHeader(const texture_bmp::InfoHeader5& infoHeader, TextureHeader& header)
{
	uint32_t orientation = 0;

	// Set the width and height
	if (infoHeader.width < 0)
	{
		header.setWidth(-1 * infoHeader.width);
		orientation |= TextureMetaData::AxisOrientationLeft;
	}
	else
	{
		header.setWidth(infoHeader.width);
		orientation |= TextureMetaData::AxisOrientationRight;
	}

	if (infoHeader.height < 0)
	{
		header.setHeight(static_cast<uint32_t>(-1 * infoHeader.height));
		orientation |= TextureMetaData::AxisOrientationDown;
	}
	else
	{
		header.setHeight(infoHeader.height);
		orientation |= TextureMetaData::AxisOrientationUp;
	}

	header.setOrientation(static_cast<TextureMetaData::AxisOrientation>(orientation));

	// The pixel format to be chosen is a little painful, but workable:
	if (infoHeader.compressionType == texture_bmp::CompressionMethod::Bitfields && infoHeader.headerSize >= texture_bmp::HeaderSize::Info2)
	{ assertion(false, "Check for gaps in the bitfields, these are invalid. A single gap at the end is ok - shove in an X channel."); }
	else if (infoHeader.compressionType == texture_bmp::CompressionMethod::AlphaBitfields && infoHeader.headerSize >= texture_bmp::HeaderSize::Info3)
	{
		assertion(false,
			" Check for gaps in the bitfields, and that the scheme can be represented by PVRTexTool. An X channel can't be put in at the end as above if there's already 4 "
			"channels.");
	}
	else
	{
		switch (infoHeader.bitsPerPixel)
		{
		case 1:
		case 2:
		case 4:
		case 8:
		{
			header.setPixelFormat(GeneratePixelType1<'l', 8>::ID);
			break;
		}
		case 16:
		{
			if (infoHeader.headerSize >= texture_bmp::HeaderSize::Info3 && infoHeader.alphaMask != 0)
			{ header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'a', 5, 5, 5, 1>::ID); }
			else
			{
				header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'x', 5, 5, 5, 1>::ID);
			}
			break;
		}
		case 24:
		{
			header.setPixelFormat(GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID);
			break;
		}
		case 32:
		{
			if (infoHeader.headerSize >= texture_bmp::HeaderSize::Info3 && infoHeader.alphaMask != 0)
			{ header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID); }
			else
			{
				header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID);
			}
			break;
		}
		}
	}

	if (infoHeader.headerSize >= texture_bmp::HeaderSize::Info5)
	{
		switch (infoHeader.colorSpace)
		{
		case texture_bmp::ColorSpace::CalibratedRGB:
		{
			// Currently, gamma correction is ignored
			break;
		}
		case texture_bmp::ColorSpace::sRGB:
		case texture_bmp::ColorSpace::Windows:
		{
			header.setColorSpace(ColorSpace::lRGB);
			break;
		}
		case texture_bmp::ColorSpace::ProfileLinked:
		case texture_bmp::ColorSpace::ProfileEmbedded:
		{
			// Currently Unsupported
			throw std::runtime_error("Embedded color profile and linked color profile not supported for BMP reader");
		}
		}
	}
}

void translateCoreHeader(const texture_bmp::CoreHeader& coreHeader, TextureHeader& header)
{
	// Set the width and height
	header.setWidth(coreHeader.width);
	header.setHeight(coreHeader.height);
	header.setPixelFormat(GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID);
	header.setOrientation(TextureMetaData::AxisOrientationUp);
}

void readImageCoreHeader(const Stream& stream, const texture_bmp::FileHeader& fileheader, const texture_bmp::CoreHeader& coreHeader, Texture& texture)
{
	// Create the texture header
	TextureHeader header;
	translateCoreHeader(coreHeader, header);
	// Create the texture from the header
	texture = Texture(header);

	// Load the image data as appropriate
	switch (coreHeader.bitsPerPixel)
	{
	case 1:
	case 4:
	case 8:
	{
		// Try to load the data
		loadIndexed(stream, fileheader, texture, static_cast<uint32_t>(texture.getBitsPerPixel() / 8), coreHeader.bitsPerPixel, (1 << coreHeader.bitsPerPixel), 4u);
		break;
	}
	case 24:
	{
		// Straightforward row reading
		loadRowAligned(stream, texture, static_cast<uint32_t>(coreHeader.bitsPerPixel / 8), 4u);
		break;
	}
	default:
	{
		throw InvalidArgumentError("Unknown number of bits per pixel for BMP reader");
	}
	}
}

void readImageInfoHeader(const Stream& stream, const texture_bmp::FileHeader& fileheader, const texture_bmp::InfoHeader5& infoHeader, Texture& texture)
{
	// Create the texture header
	TextureHeader header;
	translateInfoHeader(infoHeader, header);
	// Create the texture from the header
	texture = Texture(header);

	stream.seek(fileheader.pixelOffset, pvr::Stream::SeekOrigin::SeekOriginFromStart);

	// Check the allocation was successful
	if (texture.getDataSize() == 0) { throw InvalidArgumentError("Texture header had no data"); }

	if (infoHeader.compressionType == texture_bmp::CompressionMethod::RunLength4 || infoHeader.compressionType == texture_bmp::CompressionMethod::RunLength8)
	{ throw InvalidArgumentError("TextureReaderBMP: RunLengthEncoding not supported."); }
	if (infoHeader.compressionType != texture_bmp::CompressionMethod::None && infoHeader.compressionType != texture_bmp::CompressionMethod::Bitfields &&
		infoHeader.compressionType != texture_bmp::CompressionMethod::AlphaBitfields)
	{ throw InvalidArgumentError("TextureReaderBMP: Unknown compression type"); }
	switch (infoHeader.bitsPerPixel)
	{
	case 1:
	case 2:
	case 4:
	case 8:
	{
		// Work out the number of colors in the palette
		uint32_t numPaletteEntries = infoHeader.numColorsInTable;
		if (numPaletteEntries == 0) { numPaletteEntries = 1 << infoHeader.bitsPerPixel; }

		// Try to load the data
		loadIndexed(stream, fileheader, texture, texture.getBitsPerPixel() / 8, infoHeader.bitsPerPixel, numPaletteEntries, 4);
		break;
	}
	case 16:
	case 24:
	case 32:
	{
		// Straightforward row reading
		loadRowAligned(stream, texture, (infoHeader.bitsPerPixel / 8), 4);
		break;
	}
	default:
	{
		throw InvalidArgumentError("TextureReaderBMP: Invalid bits per pixel read.");
	}
	}
}

Texture loadImageFromFile(const Stream& stream, const texture_bmp::FileHeader& fileheader)
{
	Texture asset;
	size_t dataRead = 0;

	// Read the size of the description header, then the rest of the header.
	uint32_t headerSize = 0;
	stream.read(sizeof(headerSize), 1, &headerSize, dataRead);

	switch (headerSize)
	{
	case texture_bmp::HeaderSize::Core:
	{
		// Read the core header
		texture_bmp::CoreHeader coreHeader;
		readCoreHeader(stream, headerSize, coreHeader);
		// Translate it into a Texture
		readImageCoreHeader(stream, fileheader, coreHeader, asset);

		break;
	}
	case texture_bmp::HeaderSize::Core2:
	{
		throw InvalidOperationError("Reading from " + stream.getFileName() + " - Version 2 Core Headers are not supported.");
	}
	case texture_bmp::HeaderSize::Info1:
	case texture_bmp::HeaderSize::Info2:
	case texture_bmp::HeaderSize::Info3:
	case texture_bmp::HeaderSize::Info4:
	case texture_bmp::HeaderSize::Info5:
	{
		// Read the info header
		texture_bmp::InfoHeader5 infoHeader;
		readInfoHeader(stream, headerSize, infoHeader);
		// Translate it into a TextureHeader
		readImageInfoHeader(stream, fileheader, infoHeader, asset);
		break;
	}
	default:
	{
		throw InvalidOperationError("Reading from " + stream.getFileName() + " - Undefined image header size.");
	}
	}
	return asset;
}

} // namespace
Texture readBMP(const Stream& stream)
{
	Texture asset;
	if (!stream.isReadable()) { throw InvalidOperationError("[pvr::assetReaders::readPVR] Attempted to read a non-readable assetStream"); }

	texture_bmp::FileHeader fileHeader(readFileHeader(stream));

	// Check the Result

	uint64_t streamPosition = stream.getPosition();
	try
	{
		return loadImageFromFile(stream, fileHeader);
	}
	catch (...)
	{
		// Return to the beginning of the texture data if not loaded correctly.
		try
		{
			stream.seek((long)streamPosition, Stream::SeekOriginFromStart);
		}
		catch (...)
		{}
		throw;
	}
	// If it succeeded, let the user know that there are no more texture to load.
}

bool isBMP(const Stream& stream)
{
	// Read the magic identifier
	uint16_t magic;
	size_t dataRead;
	stream.read(sizeof(magic), 1, &magic, dataRead);

	// Make sure it read ok, if not it's probably not a usable stream.
	if (dataRead != 1) { throw FileIOError("Could not read asset stream"); }

	// Check that the identifier matches
	if (magic != texture_bmp::Identifier) { return false; }

	return true;
}

} // namespace assetReaders
} // namespace pvr
//!\endcond
