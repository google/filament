/*!
\brief Implementation of methods of the TextureWriterXNB class.
\file PVRCore/textureio/TextureReaderXNB.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/textureio/TextureReaderXNB.h"
#include "FileDefinesXNB.h"

using std::vector;
namespace pvr {
namespace assetReaders {
namespace {

void read7BitEncodedInt(const ::pvr::Stream& stream, int32_t& decodedInteger)
{
	// Check that the read operations succeed

	// Values used to decode the integer
	int32_t bitsRead = 0;
	int32_t value = 0;

	// Initialize the decoded integer
	decodedInteger = 0;

	// Loop through and read all the appropriate data to decode the integer
	do
	{
		// Read the first 7 bit value
		stream.readExact(1, 1, &value);

		// Add the bits to the decoded integer and increase the bit counter
		decodedInteger |= (value & 0x7f) << bitsRead;
		bitsRead += 7;
	} while (value & 0x80);
}

void readFileHeader(const ::pvr::Stream& stream, texture_xnb::FileHeader& xnbFileHeader)
{
	// Read the identifier
	stream.readExact(1, 3, xnbFileHeader.identifier);

	// Verify that it's an XNB header before doing anything else.
	if ((xnbFileHeader.identifier[0] != 'X') || (xnbFileHeader.identifier[1] != 'N') || (xnbFileHeader.identifier[2] != 'B'))
	{ throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + stream.getFileName() + "]: Stream was not a valid XNB"); } // Read the platform
	stream.readExact(1, 1, &xnbFileHeader.platform);
	// Read the version
	stream.readExact(1, 1, &xnbFileHeader.version);
	// Check that the version is '5' to ensure it's a supported version
	if (xnbFileHeader.version != 5) { throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + stream.getFileName() + "]: XNB Version must be 5"); }
	// Read the flags
	stream.readExact(1, 1, &xnbFileHeader.flags);
	// Read the file size
	stream.readExact(4, 1, &xnbFileHeader.fileSize);
}

void readString(const ::pvr::Stream& stream, std::string& stringToRead)
{
	// Read the std::string length
	int32_t stringLength = 0;
	read7BitEncodedInt(stream, stringLength);

	// Allocate a buffer to read in the std::string, don't forget to add a char for the NULL character.
	stringToRead.resize(static_cast<size_t>(stringLength + 1));

	// Read in the std::string data
	stream.readExact(1u, static_cast<size_t>(stringLength + 1), &stringToRead);
}

void read2DTexture(const ::pvr::Stream& stream, texture_xnb::Texture2DHeader& assetHeader, Texture& asset)
{
	// Read the surface format
	stream.readExact(sizeof(assetHeader.format), 1, &assetHeader.format);
	// Read the width
	stream.readExact(sizeof(assetHeader.width), 1, &assetHeader.width);
	// Read the height
	stream.readExact(sizeof(assetHeader.height), 1, &assetHeader.height);
	// Read the mip map count
	stream.readExact(sizeof(assetHeader.numMipMaps), 1, &assetHeader.numMipMaps);
	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(static_cast<uint64_t>(assetHeader.format)));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(static_cast<uint64_t>(assetHeader.format)));
	textureHeader.setWidth(assetHeader.width);
	textureHeader.setHeight(assetHeader.height);
	textureHeader.setNumMipMapLevels(assetHeader.numMipMaps);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32_t mipMapLevel = 0; mipMapLevel < asset.getNumMipMapLevels(); ++mipMapLevel)
	{
		// Read in the size of the next surface
		uint32_t surfaceSize = 0;
		stream.readExact(sizeof(surfaceSize), 1, &surfaceSize);
		// Make sure the surface size matches...
		if (surfaceSize != asset.getDataSize(mipMapLevel))
		{ throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + stream.getFileName() + "]: Expected data size did not match actual size"); }

		// Read in the texture data.
		stream.readExact(1, surfaceSize, asset.getDataPointer(mipMapLevel));
	}
}

void read3DTexture(const ::pvr::Stream& stream, texture_xnb::Texture3DHeader& assetHeader, Texture& asset)
{
	// Read the surface format
	stream.readExact(sizeof(assetHeader.format), 1, &assetHeader.format);
	// Read the width
	stream.readExact(sizeof(assetHeader.width), 1, &assetHeader.width);
	// Read the height
	stream.readExact(sizeof(assetHeader.height), 1, &assetHeader.height);
	// Read the depth
	stream.readExact(sizeof(assetHeader.depth), 1, &assetHeader.depth);
	// Read the mip map count
	stream.readExact(sizeof(assetHeader.numMipMaps), 1, &assetHeader.numMipMaps);
	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(static_cast<uint64_t>(assetHeader.format)));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(static_cast<uint64_t>(assetHeader.format)));
	textureHeader.setWidth(assetHeader.width);
	textureHeader.setHeight(assetHeader.height);
	textureHeader.setDepth(assetHeader.depth);
	textureHeader.setNumMipMapLevels(assetHeader.numMipMaps);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32_t mipMapLevel = 0; mipMapLevel < asset.getNumMipMapLevels(); ++mipMapLevel)
	{
		// Read in the size of the next surface
		uint32_t surfaceSize = 0;
		stream.readExact(sizeof(surfaceSize), 1, &surfaceSize);
		// Make sure the surface size matches...
		if (surfaceSize != asset.getDataSize(mipMapLevel))
		{ throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + stream.getFileName() + "]: Expected data size did not match actual size"); }

		// Read in the texture data.
		stream.readExact(1, surfaceSize, asset.getDataPointer(mipMapLevel));
	}
}

void readCubeTexture(const ::pvr::Stream& stream, texture_xnb::TextureCubeHeader& assetHeader, Texture& asset)
{
	// Read the surface format
	stream.readExact(sizeof(assetHeader.format), 1, &assetHeader.format);
	// Read the width
	stream.readExact(sizeof(assetHeader.size), 1, &assetHeader.size);
	// Read the mip map count
	stream.readExact(sizeof(assetHeader.numMipMaps), 1, &assetHeader.numMipMaps);
	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(assetHeader.format));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(assetHeader.format));
	textureHeader.setWidth(assetHeader.size);
	textureHeader.setHeight(assetHeader.size);
	textureHeader.setNumFaces(6);
	textureHeader.setNumMipMapLevels(assetHeader.numMipMaps);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32_t face = 0; face < asset.getNumFaces(); ++face)
	{
		for (uint32_t mipMapLevel = 0; mipMapLevel < asset.getNumMipMapLevels(); ++mipMapLevel)
		{
			// Read in the size of the next surface
			uint32_t surfaceSize = 0;
			stream.readExact(sizeof(surfaceSize), 1, &surfaceSize);

			// Make sure the surface size matches...
			if (surfaceSize != asset.getDataSize(mipMapLevel, false, false))
			{ throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + stream.getFileName() + "]: Expected data size did not match actual size"); }

			// Read in the texture data.
			stream.readExact(1, surfaceSize, asset.getDataPointer(mipMapLevel, 0, face));
		}
	}
}
void initializeFile(const ::pvr::Stream& stream, texture_xnb::FileHeader& fileheader, std::vector<std::string>& objectsStrings)
{
	const uint32_t c_objectNotFound = 0xffffffffu;

	// Read the file header
	readFileHeader(stream, fileheader);

	// Check if the file is compressed, if it is it's currently unsupported
	if ((fileheader.flags & texture_xnb::e_fileCompressed) != 0)
	{ throw InvalidOperationError("[TextureReaderXNB::getSupportedFileExtensions][" + stream.getFileName() + "]: Cannot load compressed XNB files - not supported."); }

	// Check the file size makes sense
	if (fileheader.fileSize != stream.getSize())
	{ throw InvalidDataError("[TextureReaderXNB::getSupportedFileExtensions][" + stream.getFileName() + "]: Data error: File size does not match stream size"); }

	// Read the number of primary objects in the file
	int32_t numAssets = 0;
	read7BitEncodedInt(stream, numAssets);

	// Resize the std::string array to hold std::string identifiers for all the assets
	objectsStrings.resize(numAssets);

	// Loop through and get all the object names
	for (int32_t assetIndex = 0; assetIndex < numAssets; ++assetIndex)
	{
		// Get the asset information
		std::string typeReaderInformation;
		readString(stream, typeReaderInformation);

		// Make sure the version is 4. something, and not incorrectly thrown in by something else.
		if (typeReaderInformation.find("Version=4") == std::string::npos)
		{ throw InvalidDataError("[TextureReaderXNB::getSupportedFileExtensions][" + stream.getFileName() + "]: Data error: Version should be 4"); } // Extract the object name
		if (typeReaderInformation.find("Microsoft.Xna.framework.content.") == std::string::npos)
		{ throw InvalidDataError("[TextureReaderXNB::getSupportedFileExtensions][" + stream.getFileName() + "]: Could not get the object name"); }

		// Extract the name of the content reader type
		size_t contentLocation = typeReaderInformation.find("Content.", 0);
		size_t typeStart = typeReaderInformation.find('.', contentLocation) + 1;
		size_t typeEnd = typeReaderInformation.find(',', typeStart);
		if (contentLocation != c_objectNotFound || typeStart != c_objectNotFound || typeEnd != c_objectNotFound)
		{
			objectsStrings[assetIndex] = typeReaderInformation;
			objectsStrings[assetIndex].erase(0, typeStart);
			objectsStrings[assetIndex].erase(typeReaderInformation.length() - typeEnd, std::string::npos);
		}

		// Get the asset version
		int32_t readerVersion = 0;
		stream.readExact(sizeof(readerVersion), 1, &readerVersion);

		// If it's not version 0, it's not supported
		if (readerVersion != 0) { throw InvalidDataError("[TextureReaderXNB::getSupportedFileExtensions][" + stream.getFileName() + "]: Reader version should be 0"); }
	}

	// Read the number of shared objects in the file
	int32_t numSharedAssets = 0;
	read7BitEncodedInt(stream, numSharedAssets);
}

} // namespace
Texture readXNB(const ::pvr::Stream& stream, int assetIndex)
{
	Texture asset;
	texture_xnb::FileHeader fileHeader;
	std::vector<std::string> objectsStrings;

	initializeFile(stream, fileHeader, objectsStrings);

	// Make sure that the next data is a texture
	if (objectsStrings[assetIndex] == "Texture2DReader")
	{
		texture_xnb::Texture2DHeader assetHeader;

		read2DTexture(stream, assetHeader, asset);
	}
	else if (objectsStrings[assetIndex] == "Texture3DReader")
	{
		texture_xnb::Texture3DHeader assetHeader;

		read3DTexture(stream, assetHeader, asset);
	}
	else if (objectsStrings[assetIndex] == "TextureCubeReader")
	{
		texture_xnb::TextureCubeHeader assetHeader;

		readCubeTexture(stream, assetHeader, asset);
	}
	else
	{
		// Don't know how to handle it.
		throw InvalidDataError("[TextureReaderXNB::readAsset_]: Could not determine the texture type - was none of 2D, 3D or Cube");
	}
	return asset;
}

bool isXNB(const Stream& assetStream)
{
	// Read the identifier
	char identifier[3];
	try
	{
		assetStream.readExact(1, 3, identifier);
	}
	catch (...)
	{
		return false;
	}
	// Reset the file

	// Check that the identifier matches
	if ((identifier[0] == 'X') && (identifier[1] == 'N') && (identifier[2] == 'B')) { return true; }

	return false;
}

uint64_t getPVRFormatFromXNBFormat(uint32_t xnbFormat)
{
	const uint64_t mappedFormats[] = {
		GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID, //???
		GeneratePixelType3<'b', 'g', 'r', 5, 6, 5>::ID,
		GeneratePixelType4<'b', 'g', 'r', 'a', 5, 5, 5, 1>::ID,
		GeneratePixelType4<'b', 'g', 'r', 'a', 4, 4, 4, 4>::ID,
		static_cast<uint64_t>(CompressedPixelFormat::DXT1),
		static_cast<uint64_t>(CompressedPixelFormat::DXT3),
		static_cast<uint64_t>(CompressedPixelFormat::DXT5),
		GeneratePixelType2<'r', 'g', 8, 8>::ID, //???
		GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID, //???
		GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID,
		GeneratePixelType2<'r', 'g', 16, 16>::ID,
		GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID,
		GeneratePixelType1<'a', 8>::ID,
		GeneratePixelType1<'r', 32>::ID,
		GeneratePixelType2<'r', 'g', 32, 32>::ID,
		GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID,
		GeneratePixelType1<'r', 16>::ID,
		GeneratePixelType2<'r', 'g', 16, 16>::ID,
		GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID,
		GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID,
	};
	return mappedFormats[xnbFormat];
}

VariableType getPVRTypeFromXNBFormat(uint32_t xnbFormat)
{
	const VariableType mappedTypes[] = { VariableType::UnsignedByteNorm, VariableType::UnsignedShortNorm, VariableType::UnsignedShortNorm, VariableType::UnsignedShortNorm,
		VariableType::UnsignedByteNorm, VariableType::UnsignedByteNorm, VariableType::UnsignedByteNorm, VariableType::UnsignedByteNorm, VariableType::UnsignedByteNorm,
		VariableType::UnsignedIntegerNorm, VariableType::UnsignedShortNorm, VariableType::UnsignedShortNorm, VariableType::UnsignedByteNorm, VariableType::SignedFloat,
		VariableType::SignedFloat, VariableType::SignedFloat, VariableType::SignedFloat, VariableType::SignedFloat, VariableType::SignedFloat, VariableType::SignedFloat };

	return mappedTypes[xnbFormat];
}

} // namespace assetReaders
} // namespace pvr
//!\endcond
