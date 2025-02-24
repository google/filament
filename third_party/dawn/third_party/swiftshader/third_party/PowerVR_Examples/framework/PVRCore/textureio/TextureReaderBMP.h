/*!
\brief An experimental BMP texture reader.
\file PVRCore/textureio/TextureReaderBMP.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/Stream.h"

namespace pvr {
/// <summary>Contains classes whose purpose is to read specific storage formats (bmp, POD, pfx, pvr etc.) into
/// PVRAssets classes (Texture, Model, Effect etc.).</summary>
namespace assetReaders {
/// <summary>Experimental BMP Texture reader</summary>
Texture readBMP(const Stream& stream);

bool isBMP(const Stream& stream);

} // namespace assetReaders
} // namespace pvr
