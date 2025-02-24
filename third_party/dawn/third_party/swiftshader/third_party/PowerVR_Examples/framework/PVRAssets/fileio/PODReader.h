/*!
\brief An AssetReader that reads POD format streams and creates pvr::assets::Model objects out of them.
\file PVRAssets/fileio/PODReader.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRAssets/Model.h"
#include "PVRCore/stream/Stream.h"

namespace pvr {
namespace assets {

/// <summary>This class creates pvr::assets::Model object from Streams of POD Model data. Use the readAsset method
/// to create Model objects from the data in your stream.</summary>
::pvr::assets::Model readPOD(const ::pvr::Stream& stream);

/// <summary>This class creates pvr::assets::Model object from Streams of POD Model data. Use the readAsset method
/// to create Model objects from the data in your stream.</summary>
void readPOD(const ::pvr::Stream& stream, ::pvr::assets::Model& model);

/// <summary>Check if this reader supports the particular assetStream.</summary>
/// <param name="assetStream">The stream to check</param>
/// <returns>True if this reader supports the particular assetStream</returns>
bool isPOD(const ::pvr::Stream& stream);

} // namespace assets
} // namespace pvr
