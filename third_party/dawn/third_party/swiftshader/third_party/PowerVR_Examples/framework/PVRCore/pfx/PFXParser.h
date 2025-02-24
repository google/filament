/*!
\brief An AssetReader that parses and reads PFX effect files into pvr::assets::Effect objects.
\file PVRCore/pfx/PFXParser.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/IAssetProvider.h"
#include "PVRCore/pfx/Effect.h"

namespace pvr {
// Forward Declarations
namespace pfx {

/// <summary>PFX reader.</summary>
effect::Effect readPFX(const ::pvr::Stream& stream, const IAssetProvider* assetProvider);

/// <summary>PFX reader.</summary>
void readPFX(const ::pvr::Stream& stream, const IAssetProvider* assetProvider, effect::Effect& outEffect);

} // namespace pfx

} // namespace pvr
