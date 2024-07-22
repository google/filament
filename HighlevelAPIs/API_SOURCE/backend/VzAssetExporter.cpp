#pragma once
#include "VzAssetExporter.h"

#include <fstream>

#include "../VzEngineApp.h"
#define CGLTF_WRITE_IMPLEMENTATION
#include <cgltf_write.h>
#include <viewer/Settings.h>

extern vzm::VzEngineApp gEngineApp;

namespace filament::gltfio {

using Settings = filament::viewer::Settings;
using JsonSerializer = filament::viewer::JsonSerializer;

void VzAssetExpoter::ExportToGlb(const vzm::VzAsset* v_asset,
                                 const std::string path) {
  if (v_asset == nullptr) {
    return;
  }
  AssetVID vid_asset = v_asset->GetVID();
  VzAssetRes& asset_res = *gEngineApp.GetAssetRes(vid_asset);

  FFilamentAsset* fasset = downcast(asset_res.asset);
  cgltf_data* data = (cgltf_data*)fasset->getSourceAsset();
  std::string output;
  cgltf_options options{};
  cgltf_size buffer_size = cgltf_write(&options, nullptr, 0, data);
  output.resize(buffer_size);

  cgltf_write(&options, output.data(), buffer_size, data);

  std::ofstream file(path.c_str(), std::ios::binary);
  if (file) {
    int version_num = 2;
    int align = 4 - (output.length() - 1) % 4;
    int size = output.length() + 27 + data->bin_size + align;
    int bin_magic = 0x004E4942;

    file.write("glTF", 4);
    file.write(reinterpret_cast<char*>(&version_num), 4);
    file.write(reinterpret_cast<char*>(&size), 4);

    size = output.length() - 1 + align;
    file.write(reinterpret_cast<char*>(&size), 4);
    file.write("JSON", 4);
    file.write(output.c_str(), output.length() - 1);

    for (int k = 0; k < align; k++) {
      file.write(" ", 1);
    }

    file.write(reinterpret_cast<char*>(&data->bin_size), 4);
    file.write(reinterpret_cast<char*>(&bin_magic), 4);
    file.write(reinterpret_cast<const char*>(data->bin), data->bin_size);
    file.close();
  }
}

}  // namespace filament::gltfio
