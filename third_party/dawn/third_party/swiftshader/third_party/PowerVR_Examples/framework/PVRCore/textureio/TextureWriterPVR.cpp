/*!
\brief Implementation of methods of the TextureWriterPVR class.
\file PVRCore/textureio/TextureWriterPVR.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/textureio/TextureWriterPVR.h"
#include "PVRCore/textureio/FileDefinesPVR.h"
#include "PVRCore/Log.h"
using std::map;
using std::vector;
namespace pvr {
namespace assetWriters {
inline static void writeTextureMetaDataToStream(Stream& stream, const TextureMetaData& metadata)
{
	auto fourCC = metadata.getFourCC();
	auto key = metadata.getKey();
	auto dataSize = metadata.getDataSize();
	stream.writeExact(sizeof(fourCC), 1, &fourCC);
	stream.writeExact(sizeof(key), 1, &key);
	stream.writeExact(sizeof(dataSize), 1, &dataSize);

	stream.writeExact(1, dataSize, metadata.getData());
}

inline static void writeTextureMetaDataToStream(Stream&& stream, const TextureMetaData& metadata)
{
	Stream& str = stream;
	writeTextureMetaDataToStream(str, metadata);
}

void writePVR(const Texture& asset, Stream& stream)
{
	// Check the size of data written.
	uint32_t version = TextureHeader::PVRv3;

	stream.writeExact(sizeof(version), 1, &version); // Write the texture header version
	stream.writeExact(sizeof(asset.flags), 1, &asset.flags); // Write the flags
	stream.writeExact(sizeof(asset.pixelFormat), 1, &asset.pixelFormat); // Write the pixel format
	stream.writeExact(sizeof(asset.colorSpace), 1, &asset.colorSpace); // Write the color space
	stream.writeExact(sizeof(asset.channelType), 1, &asset.channelType); // Write the channel type
	stream.writeExact(sizeof(asset.height), 1, &asset.height); // Write the height
	stream.writeExact(sizeof(asset.width), 1, &asset.width); // Write the width
	stream.writeExact(sizeof(asset.depth), 1, &asset.depth); // Write the depth
	stream.writeExact(sizeof(asset.numSurfaces), 1, &asset.numSurfaces); // Write the number of surfaces
	stream.writeExact(sizeof(asset.numFaces), 1, &asset.numFaces); // Write the number of faces
	stream.writeExact(sizeof(asset.numMipMaps), 1, &asset.numMipMaps); // Write the number of MIP maps
	stream.writeExact(sizeof(asset.metaDataSize), 1, &asset.metaDataSize); // Write the meta data size

	// Write the meta data
	const map<uint32_t, map<uint32_t, TextureMetaData>>* metaDataMap = asset.getMetaDataMap();
	map<uint32_t, map<uint32_t, TextureMetaData>>::const_iterator walkMetaDataMap = metaDataMap->begin();
	for (; walkMetaDataMap != metaDataMap->end(); ++walkMetaDataMap)
	{
		const map<uint32_t, TextureMetaData>& currentDevMetaDataMap = walkMetaDataMap->second;
		map<uint32_t, TextureMetaData>::const_iterator walkCurDevMetaMap = currentDevMetaDataMap.begin();
		for (; walkCurDevMetaMap != currentDevMetaDataMap.end(); ++walkCurDevMetaMap) { writeTextureMetaDataToStream(stream, walkCurDevMetaMap->second); }
	}

	// Write the texture data
	stream.writeExact(1, asset.getDataSize(), asset.getDataPointer());
}

void writePVR(const Texture& asset, Stream&& stream)
{
	Stream& str = stream;
	writePVR(asset, str);
}

} // namespace assetWriters
} // namespace pvr
//!\endcond
