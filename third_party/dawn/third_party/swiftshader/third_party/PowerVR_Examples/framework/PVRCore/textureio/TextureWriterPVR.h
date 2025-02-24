/*!
\brief An experimental Writer that writes pvr::asset::Texture objects into a PVR file.
\file PVRCore/textureio/TextureWriterPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/Stream.h"

namespace pvr {
namespace assetWriters {
void writePVR(const ::pvr::Texture& texture, ::pvr::Stream& stream);
void writePVR(const ::pvr::Texture& texture, ::pvr::Stream&& stream);
} // namespace assetWriters
} // namespace pvr
