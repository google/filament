/*!
\brief Implementation file for the AndroidAssetStream.
\file PVRCore/Android/AndroidAssetStream.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCore/Android/AndroidAssetStream.h"
#include <android/asset_manager.h>

namespace pvr {

void AndroidAssetStream::_read(size_t size, size_t count, void* outData, size_t& outElementsRead) const
{
	int dataRead = (size_t)AAsset_read(_asset, outData, size * count);

	if (dataRead == 0) { outElementsRead = dataRead; }
	else if (dataRead < 0)
	{
		outElementsRead = 0;
		throw FileIOError(getFileName(), "[AndroidAssetStream::read] Unknown Error.");
	}
	// AssetManager returns number of bytes. We want number of items.
	outElementsRead = (size_t)dataRead / size;
}

void AndroidAssetStream::_write(size_t size, size_t count, const void* data, size_t& dataWritten)
{
	throw FileIOError(getFileName(), "[AndroidAssetStream::write]: Android Asset Stream are not writeable.");
}

void AndroidAssetStream::_seek(long offset, SeekOrigin origin) const
{
	off_t newPos = AAsset_seek(_asset, offset, (int)origin);

	if (newPos == (off_t)-1) { throw FileIOError(getFileName(), "[AndroidAssetStream::seek] Attempt to seek  past the end of stream."); }
}

void AndroidAssetStream::open() const
{
	_asset = AAssetManager_open(assetManager, _fileName.c_str(), AASSET_MODE_RANDOM);
	if (!_asset) { throw FileNotFoundError(getFileName(), "[AndroidAssetStream::open] Attempted to open a nonexistent file"); }
}

void AndroidAssetStream::close()
{
	AAsset_close(_asset);
	_asset = NULL;
}

uint64_t AndroidAssetStream::_getPosition() const
{
	if (_asset) { return static_cast<uint64_t>(AAsset_getLength(_asset) - AAsset_getRemainingLength(_asset)); }

	return 0;
}

uint64_t AndroidAssetStream::_getSize() const
{
	if (_asset) { return static_cast<uint64_t>(AAsset_getLength(_asset)); }
	return 0;
}
} // namespace pvr