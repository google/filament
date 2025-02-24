/*!
\brief An experimental KTX texture reader.
\file PVRCore/textureio/TextureReaderKTX.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/Stream.h"

namespace pvr {
namespace assetReaders {

Texture readKTX(const Stream& stream);

} // namespace assetReaders
} // namespace pvr
