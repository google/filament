#pragma once
#include "PVRAssets/Model.h"
#include "PVRCore/stream/Stream.h"
#include "PVRCore/IAssetProvider.h"

namespace pvr {
namespace assets {

void readGLTF(const ::pvr::Stream& stream, const IAssetProvider& assetProvider, ::pvr::assets::Model& outModel);

::pvr::assets::Model readGLTF(const ::pvr::Stream& stream, const IAssetProvider& assetProvider);

} // namespace assets
} // namespace pvr
