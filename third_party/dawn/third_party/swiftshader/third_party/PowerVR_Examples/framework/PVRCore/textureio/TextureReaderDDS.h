/*!
\brief An experimental DDS texture reader.
\file PVRCore/textureio/TextureReaderDDS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/Stream.h"

namespace pvr {
namespace assetReaders {
/// <summary>Experimental DDS Texture reader</summary>
Texture readDDS(const ::pvr::Stream& stream);

} // namespace assetReaders
} // namespace pvr
