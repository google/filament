#pragma once
#include "../FIncludes.h"

namespace vzm {

struct VzAsset;

}

namespace filament::gltfio {

struct VzAssetExpoter {

  void ExportToGlb(const vzm::VzAsset* v_asset, const std::string path);

};

}

