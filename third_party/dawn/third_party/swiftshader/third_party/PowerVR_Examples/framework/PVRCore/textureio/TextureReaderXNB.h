/*!
\brief An experimental AssetReader that reads pvr::asset::Texture objects from an XNB file.
\file PVRCore/textureio/TextureReaderXNB.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/Stream.h"

namespace pvr {
class Stream;
namespace assetReaders {
/// <summary>Experimental XNB Texture reader</summary>
Texture readXNB(const ::pvr::Stream& stream, int assetIndex = 0);
VariableType getPVRTypeFromXNBFormat(uint32_t xnbFormat);
uint64_t getPVRFormatFromXNBFormat(uint32_t xnbFormat);
} // namespace assetReaders
} // namespace pvr
