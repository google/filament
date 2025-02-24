/*!
\brief Implementation of methods of the Texture class.
\file PVRCore/texture/Texture.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/texture/Texture.h"
#include "PVRCore/Errors.h"
#include <algorithm>

namespace pvr {

TextureFileFormat getTextureFormatFromFilename(const char* assetname)
{
	std::string file(assetname);

	size_t period = file.rfind(".");
	if (period != std::string::npos)
	{
		std::string s = file.substr(period + 1);
		std::transform(s.begin(), s.end(), s.begin(), [](char c) { return static_cast<char>(tolower(c)); });
		if (!s.compare("pvr")) { return TextureFileFormat::PVR; }
		if (!s.compare("tga")) { return TextureFileFormat::TGA; }
		if (!s.compare("ktx")) { return TextureFileFormat::KTX; }
		if (!s.compare("bmp")) { return TextureFileFormat::BMP; }
		if (!s.compare("dds")) { return TextureFileFormat::DDS; }
		if (!s.compare("ddx")) { return TextureFileFormat::DDX; }
	}
	return TextureFileFormat::UNKNOWN;
}

uint8_t Texture::getPixelSize() const { return pixelFormat.getBitsPerPixel() / 8u; }

Texture::Texture() { _pTextureData.resize(getDataSize()); }

Texture::Texture(const TextureHeader& sHeader, const unsigned char* pData) : TextureHeader(sHeader)
{
	uint32_t sizeOfData = getDataSize();
	// Allocate new memory for the texture.
	_pTextureData.resize(sizeOfData);

	// If there is data supplied, copy it into this texture.
	if (pData && sizeOfData) { memcpy(_pTextureData.data(), pData, sizeOfData); }
}

void Texture::initializeWithHeader(const TextureHeader& sHeader)
{
	*this = sHeader;
	// Get the data size from the newly attached header.
	_pTextureData.resize(getDataSize());
}

const unsigned char* Texture::getDataPointer(uint32_t mipMapLevel /*= 0*/, uint32_t arrayMember /*= 0*/, uint32_t face /*= 0*/) const
{
	if ((static_cast<int32_t>(mipMapLevel) == pvrTextureAllMipMaps) || mipMapLevel >= getNumMipMapLevels())
	{ throw InvalidArgumentError("mipmapLevel", "Texture::getDataPointer: Specified mipmap level did not exist"); }
	if (arrayMember >= getNumArrayMembers()) { throw InvalidArgumentError("arrayMember", "Texture::getDataPointer: Specified array member did not exist"); }
	if (face >= getNumFaces()) { throw InvalidArgumentError("face", "Texture::getDataPointer: Specified face did not exist"); }
	uint32_t offSet = 0;
	// File is organised by MIP Map levels, then surfaces, then faces.

	// Get the start of the MIP level.
	if (mipMapLevel != 0)
	{
		// Get the size for all MIP Map levels up to this one.
		for (uint32_t uiCurrentMipMap = 0; uiCurrentMipMap < mipMapLevel; ++uiCurrentMipMap) { offSet += getDataSize(static_cast<int32_t>(uiCurrentMipMap), true, true); }
	}

	// Get the start of the array.
	if (arrayMember != 0) { offSet += arrayMember * getDataSize(static_cast<int32_t>(mipMapLevel), false, true); }

	// Get the start of the face.
	if (face != 0) { offSet += face * getDataSize(static_cast<int32_t>(mipMapLevel), false, false); }

	// Return the data pointer plus whatever offSet has been specified.
	return &_pTextureData[offSet];
}

unsigned char* Texture::getDataPointer(uint32_t mipMapLevel /*= 0*/, uint32_t arrayMember /*= 0*/, uint32_t face /*= 0*/)
{
	// Initialize the offSet value.
	uint32_t offSet = 0;

	// Error checking
	if ((static_cast<int32_t>(mipMapLevel) == pvrTextureAllMipMaps) || mipMapLevel >= getNumMipMapLevels())
	{ throw InvalidArgumentError("mipmapLevel", "Texture::getDataPointer: Specified mipmap level did not exist"); }
	if (arrayMember >= getNumArrayMembers()) { throw InvalidArgumentError("arrayMember", "Texture::getDataPointer: Specified array member did not exist"); }
	if (face >= getNumFaces())
	{ throw InvalidArgumentError("face", "Texture::getDataPointer: Specified face did not exist"); } // File is organised by MIP Map levels, then surfaces, then faces.

	// Get the start of the MIP level.
	if (mipMapLevel != 0)
	{
		// Get the size for all MIP Map levels up to this one.
		for (uint32_t uiCurrentMipMap = 0; uiCurrentMipMap < mipMapLevel; ++uiCurrentMipMap) { offSet += getDataSize(static_cast<int32_t>(uiCurrentMipMap), true, true); }
	}

	// Get the start of the array.
	if (arrayMember != 0) { offSet += arrayMember * getDataSize(mipMapLevel, false, true); }

	// Get the start of the face.
	if (face != 0) { offSet += face * getDataSize(mipMapLevel, false, false); }

	// Return the data pointer plus whatever offSet has been specified.
	return &_pTextureData[offSet];
}

void Texture::addPaddingMetaData(uint32_t paddingAlignment)
{
	// If the alignment is 0 or 1, return - as nothing is required
	if (paddingAlignment <= 1) { return; }

	// Set the meta data padding. The 12 is the size of an empty meta data block
	uint32_t unpaddedStartOfTextureData = (SizeOfHeader + getMetaDataSize() + 12);

	// Work out the value of the padding
	uint32_t paddingAmount = ((static_cast<uint32_t>(-1) * unpaddedStartOfTextureData) % paddingAlignment);

	// Create the meta data
	TextureMetaData metaPadding(PVRv3, TextureMetaData::IdentifierPadding, paddingAmount, NULL);

	// Add the meta data to the texture
	addMetaData(metaPadding);
}

} // namespace pvr
//!\endcond
