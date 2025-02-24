/*!
\brief An experimental AssetReader that reads pvr::asset::Texture objects from an TGA file.
\file PVRCore/textureio/TextureReaderTGA.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/textureio/FileDefinesTGA.h"
#include "PVRCore/stream/Stream.h"

namespace pvr {
namespace assetReaders {

Texture readTGA(const Stream& str);
} // namespace assetReaders
} // namespace pvr
